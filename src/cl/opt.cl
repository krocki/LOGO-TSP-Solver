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

/*
 * //mutex lock void inline lock( volatile __global int* mutex ) { while(
 * atomic_cmpxchg( mutex, 0, 1 ) != 0); }
 *
 * //mutex unlock void inline unlock( volatile __global int* mutex ) { atomic_xchg(
 * mutex, 0 ); }
 */

int inline
calculateDistance2DSimple (unsigned int i, unsigned int j,
                           __local city_coords * localCoords) {
    float           dx,
                    dy;
    dx = localCoords[i].x - localCoords[j].x;
    dy = localCoords[i].y - localCoords[j].y;
    return (int) (native_sqrt (dx * dx + dy * dy) + 0.5f);
}

int inline
calculate_change (unsigned int i, unsigned int j, __local city_coords * coords) {
    /*
     * Vectorized code
     */
    float4          dx1,
                    dy1,
                    dx2,
                    dy2,
                    dx,
                    dy;
    dx1 = (float4) (coords[j + 1].x, coords[i - 1].x, coords[i].x, coords[j].x);
    dx2 = (float4) (coords[i].x, coords[j].x, coords[i - 1].x, coords[j + 1].x);
    dy1 = (float4) (coords[j + 1].y, coords[i - 1].y, coords[i].y, coords[j].y);
    dy2 = (float4) (coords[i].y, coords[j].y, coords[i - 1].y, coords[j + 1].y);
    dx = dx1 - dx2;
    dy = dy1 - dy2;
    dx = native_sqrt (dx * dx + dy * dy) + 0.5f;
    return (int) dx.s0 + (int) dx.s1 - (int) dx.s2 - (int) dx.s3;
}

// #define BLOCK_SIZE 256

__kernel void
cl2optKernel (__global const city_coords * dCoords,
              __global struct best2_out *dResult,
              __local city_coords * localCoords,
              __local best2_out * best_values,
              int BLOCK_SIZE, unsigned int cities,
              unsigned long number_of_2opts, unsigned int iterations) {
    // unsigned globalId = get_global_id(0);
    // unsigned localId = get_local_id(0);
    // unsigned localSize = get_local_size(0);
    // unsigned globalSize = get_global_size(0);
    struct best2_out o;
    int             k;
    int             id;
    // __local city_coords localCoords[2560];
    // __local best2_out best_values[BLOCK_SIZE];
    ROUTE_DATA_TYPE localCities;
    unsigned int    iter;
    unsigned long   max;
    unsigned int    i,
             j;
    unsigned int    blocks;
    int             change;
    max = number_of_2opts;
    iter = iterations;
    localCities = cities;
    o.i = 0;
    o.j = 0;
    o.minchange = 99999;
#pragma unroll

    for (k = get_local_id (0); k < localCities; k += get_local_size (0) ) {
        localCoords[k] = dCoords[k];
    }

    // async_work_group_copy(localCoords, dCoords, 3072, 0);
    barrier (CLK_LOCAL_MEM_FENCE);
#pragma unroll

    for (int no = 0; no < iter; no++) {
        id = get_global_id (0) + no * get_global_size (0);

        if (id < max) {
            // a nasty formula to calculate the right cell of a
            // triangular matrix index based on the thread id
            j = (unsigned int) (3 + native_sqrt (8.0f * (float) id + 1.0f) ) / 2;	// l_idx.i;
            i = id - (j - 2) * (j - 1) / 2 + 1;
            // calculate the effect of (i,j) swap
            change = calculate_change (i, j, localCoords);

            // calculateDistance2DSimple(i, j + 1, localCoords) +
            // calculateDistance2DSimple(i - 1, j, localCoords)
            // - calculateDistance2DSimple(i, i - 1, localCoords) -
            // calculateDistance2DSimple(j + 1, j, localCoords);

            // save if best than already known swap
            if (change < o.minchange) {
                o.minchange = change;
                o.i = i;
                o.j = j + 1;
            }
        }
    }

    // intra-block reduction...
    // blocks = BLOCK_SIZE/2;
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

    /*
     * inter block reduction done by the CPU
     */
    /*
     * maybe it's cheaper to do that on CPU for several blocks
     */
}
