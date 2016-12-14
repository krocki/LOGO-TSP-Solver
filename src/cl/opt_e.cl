/*
 *   Logo TSP Solver ver. 0.62  Copyright (C) 2013  Kamil Rocki
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_amd_printf : enable

#define ROUTE_DATA_TYPE unsigned int

typedef struct cc {

    float           x;
    float           y;

} city_coords;

typedef struct best2_out {

    ROUTE_DATA_TYPE i;
    ROUTE_DATA_TYPE j;
    int             minchange;

} best2_out;

#define mod128(x) (x & 127)

/*
 * NOT Vectorized code - debug
 */
int inline
calculateDistance2D_extended (unsigned int i, unsigned int j,
                              __local city_coords * coordsA,
                              __local city_coords * coordsB) {
    float           dx,
                    dy;
    dx = coordsA[i].x - coordsB[j].x;
    dy = coordsA[i].y - coordsB[j].y;
    // dx = dx * dx;
    // dx = fma(dy, dy, dx);
    return (int) (sqrt (dx * dx + dy * dy) + 0.5f);
}

int inline
calculate_change (unsigned int i, unsigned int j, __local city_coords * coordsA,
                  __local float2 * coordsB) {
    /*
     * Vectorized code
     */
    float4          dx1,
                    dy1,
                    dx2,
                    dy2,
                    dx,
                    dy;
    dx1 = (float4) (coordsA[j + 1].x, coordsB[i].x, coordsB[i].x, coordsA[j].x);
    dy1 = (float4) (coordsA[j + 1].y, coordsB[i].y, coordsB[i].y, coordsA[j].y);
    dx2 =
        (float4) (coordsB[i + 1].x, coordsA[j].x, coordsB[i + 1].x,
                  coordsA[j + 1].x);
    dy2 =
        (float4) (coordsB[i + 1].y, coordsA[j].y, coordsB[i + 1].y,
                  coordsA[j + 1].y);
    dx = dx1 - dx2;
    dy = dy1 - dy2;
    dx = native_sqrt (dx * dx + dy * dy) + 0.5f;
    return (int) dx.s0 + (int) dx.s1 - (int) dx.s2 - (int) dx.s3;
}


// #define BLOCK_SIZE 256
// #define MAX_COORDS_EXTENDED_CL 1792 //1792
// #define ITERS_INSIDE 14
// /MAX_COORDS_EXTENDED_CL/sqrt(number of blocks * number of threads)
// here 1536/128
#define UNROLL_ITERS_INSIDE 16
// #define MANUAL_UNROLL

#define inner_loop(X)  for (re_j = 0; re_j < ITERS_INSIDE; re_j++) { \
        i = (get_global_id(0) >> 7) + ((X) << 7) ; \
        j = mod128(get_global_id(0))  + (re_j << 7)  ; \
        if (i + b > j + a + 1) { \
            change =  \
                      calculateDistance2D_extended(j + 1, i + 1,  coords_extended_A, coords_extended_B) + \
                      calculateDistance2D_extended(i,     j,      coords_extended_B, coords_extended_A) - \
                      calculateDistance2D_extended(i,     i + 1,  coords_extended_B, coords_extended_B) - \
                      calculateDistance2D_extended(j,     j + 1,  coords_extended_A, coords_extended_A); \
            if (change < o.minchange) { \
                o.minchange = change; \
                o.i = i + rB_start + 1; \
                o.j = j + rA_start + 1;   \
            } \
        } \
    }


__kernel __attribute__ ( (vec_type_hint (float4) ) )
void            cl2optKernel_e (__global const city_coords * dCoords,
                                __global struct best2_out *dResult,
                                __local city_coords * coords_extended_A,
                                __local city_coords * coords_extended_B,
                                __local best2_out * best_values,
                                int MAX_COORDS_EXTENDED_CL, int BLOCK_SIZE,
                                int ITERS_INSIDE, ROUTE_DATA_TYPE c) {
    struct best2_out o;
    int             i,
                    j;
    int             change;
    int             rA_start,
                    rB_start,
                    rA_end,
                    rB_end;
    unsigned int    id = get_global_id (0);
    int             re_i,
                    re_j;
    unsigned int    blocks;
    // 2 ranges of coordinates
    // __local city_coords coords_extended_A[1792+1];
    // __local city_coords coords_extended_B[1792+1];
    // __local best2_out best_values[256];
    ROUTE_DATA_TYPE cities;
    int             iters_inside_a_blockB = ITERS_INSIDE;
    cities = c;
    o.i = 1;
    o.j = 1;
    o.minchange = 99999;
    barrier (CLK_LOCAL_MEM_FENCE);
    int             const1 = cities - MAX_COORDS_EXTENDED_CL;
    int             const2 =
        (cities - (cities / MAX_COORDS_EXTENDED_CL) * MAX_COORDS_EXTENDED_CL) / 128 +
        1;

    for (int a = 1; a < cities; a += MAX_COORDS_EXTENDED_CL) {
        a = min (a, const1);
        rA_start = a - 1;
        rA_end = a + MAX_COORDS_EXTENDED_CL;	// Vertical size

        for (int k = rA_start + get_local_id (0); k < rA_end; k += get_local_size (0) ) {
            coords_extended_A[k - rA_start] = dCoords[k];
        }

        for (int b = const1; b > a - MAX_COORDS_EXTENDED_CL;
                b -= MAX_COORDS_EXTENDED_CL) {
            if ( (b < a) || (a == const1) ) {
                b = a;
            }

            rB_start = b - 1;
            rB_end = b + MAX_COORDS_EXTENDED_CL;

            for (int t = rB_start + get_local_id (0); t < rB_end;
                    t += get_local_size (0) ) {
                coords_extended_B[t - rB_start] = dCoords[t];
            }

            barrier (CLK_LOCAL_MEM_FENCE);
            iters_inside_a_blockB = ITERS_INSIDE;

            // check if there are some subblocks that can be skipped
            if (b == a)
                if (a != const1) {
                    iters_inside_a_blockB = const2;
                }

            // manually unrolled
#ifndef MANUAL_UNROLL
#pragma unroll UNROLL_ITERS_INSIDE

            for (re_i = 0; re_i < iters_inside_a_blockB; re_i++) {
#pragma unroll UNROLL_ITERS_INSIDE

                for (re_j = 0; re_j < ITERS_INSIDE; re_j++) {
                    // 128x128 block, if different this has to be changed
                    i = (get_global_id (0) >> 7) + (re_i << 7);
                    j = mod128 (get_global_id (0) ) + (re_j << 7);

                    // only triangular matrix
                    if (i + b > j + a + 1) {
                        // calculate the effect of (i,j) swap
                        change =
                            calculate_change (i, j, coords_extended_A,
                                              coords_extended_B);

                        // calculateDistance2D_extended(j + 1, i + 1,
                        // coords_extended_A, coords_extended_B) +
                        // calculateDistance2D_extended(i, j, coords_extended_B,
                        // coords_extended_A) -
                        // calculateDistance2D_extended(i, i + 1, coords_extended_B,
                        // coords_extended_B) -
                        // calculateDistance2D_extended(j, j + 1, coords_extended_A,
                        // coords_extended_A);

                        if (change < o.minchange) {
                            o.minchange = change;
                            o.i = i + rB_start + 1;
                            o.j = j + rA_start + 1;
                        }
                    }
                }
            }

#else
            re_i = 0;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

            re_i++;

            if (iters_inside_a_blockB > re_i) {
#pragma unroll UNROLL_ITERS_INSIDE
                inner_loop (re_i)
            }

#endif
            barrier (CLK_LOCAL_MEM_FENCE);
        }
    }

    best_values[get_local_id (0)] = o;
    barrier (CLK_LOCAL_MEM_FENCE);
#pragma unroll

    for (blocks = BLOCK_SIZE / 2; blocks >= 1; blocks = blocks / 2) {
        if (get_local_id (0) < blocks
                && best_values[get_local_id (0) + blocks].minchange < 0) {
            if (best_values[get_local_id (0)].minchange >
                    best_values[get_local_id (0) + blocks].minchange) {
                best_values[get_local_id (0)].minchange =
                    best_values[get_local_id (0) + blocks].minchange;
                best_values[get_local_id (0)].i =
                    best_values[get_local_id (0) + blocks].i;
                best_values[get_local_id (0)].j =
                    best_values[get_local_id (0) + blocks].j;
            }
        }

        barrier (CLK_LOCAL_MEM_FENCE);
    }

    if (get_local_id (0) == 0) {
        dResult[get_group_id (0)] = best_values[get_local_id (0)];
    }
}
