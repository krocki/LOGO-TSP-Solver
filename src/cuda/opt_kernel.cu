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

#include <headers.h>
#include <cuda_common.h>
#include <cuda_defs.h>

#ifdef LEGACY_GPUS
#define REDUCE_ON_CPU
#endif

#ifndef USE_ORDERED_COORDS
__launch_bounds__ (1024, 32)
__global__ void kernel2opt (city_coords * device_coords, int _cities,
                            ROUTE_DATA_TYPE * device_route,
                            unsigned long number_of_2opts,
                            unsigned int iterations, int *mutex,
                            struct best2_out *best_2opt) {
#else
__launch_bounds__ (1024, 32)
__global__ void kernel2opt (city_coords * device_coords, int _cities,
                            unsigned long number_of_2opts,
                            unsigned int iterations, int *mutex,
                            struct best2_out *best_2opt) {
#endif
    register int    local_id = threadIdx.x + blockIdx.x * blockDim.x;
    register int    id;
    register unsigned int i,
             j;
    register unsigned long max = number_of_2opts;
    // 2-opt move index
    register int    change;
    register int    packSize = blockDim.x * gridDim.x;
    struct best2_out best;
    register int    iter = iterations;
    best.minchange = 999999;
#ifdef LEGACY_GPUS
    __shared__ city_coords coords[CUDA_MAX_SIZE_SIMPLE_LEGACY];
#else
    __shared__ city_coords coords[CUDA_MAX_SIZE_SIMPLE];
#endif
    __shared__ int  cities;
#ifdef LEGACY_GPUS
    __shared__ best2_out best_values[256];
#else
    __shared__ best2_out best_values[1024];
#endif
    // copy to a local register
    cities = _cities;
    // copy the coordinates of all route points to the shared memory
#ifndef USE_ORDERED_COORDS

    for (register int k = threadIdx.x; k < cities; k += blockDim.x) {
        coords[k] = device_coords[device_route[k]];
    }

#else

    for (register int k = threadIdx.x; k < cities; k += blockDim.x) {
        coords[k] = device_coords[k];
    }

#endif
    __syncthreads();
    // each thread performs iter inner iterations in order to reuse the shared
    // memory
#pragma unroll

    for (register int no = 0; no < iter; no++) {
        id = local_id + no * packSize;

        if (id < max) {
            // a nasty formula to calculate the right cell of a triangular matrix
            // index based on the thread id
            j = (unsigned int) (3 + __fsqrt_rn (8.0f * (float) id + 1.0f) ) / 2;	// l_idx.i;
            i = id - (j - 2) * (j - 1) / 2 + 1;
            // calculate the effect of (i,j) swap
            change =
                calculateDistance2DSimple (i, j + 1,
                                           coords) + calculateDistance2DSimple (i - 1,
                                                   j,
                                                   coords)
                - calculateDistance2DSimple (i, i - 1,
                                             coords) - calculateDistance2DSimple (j +
                                                     1, j,
                                                     coords);

            // save if best than already known swap
            if (change < best.minchange) {
                best.minchange = change;
                best.i = i;
                best.j = j + 1;
            }
        }
    }

    // intra-block reduction...it doesn't look pretty, but works
    best_values[threadIdx.x] = best;
    __syncthreads();
#ifndef LEGACY_GPUS

    // 1024 threads
    if (threadIdx.x < 512)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 512].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 512].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 512].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 512].j;
        }

    __syncthreads();

    // 512 threads
    if (threadIdx.x < 256)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 256].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 256].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 256].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 256].j;
        }

    __syncthreads();
#endif

    // 256 threads
    if (threadIdx.x < 128)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 128].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 128].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 128].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 128].j;
        }

    __syncthreads();

    // 128 threads
    if (threadIdx.x < 64)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 64].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 64].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 64].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 64].j;
        }

    __syncthreads();

    // 64 threads
    if (threadIdx.x < 32)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 32].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 32].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 32].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 32].j;
        }

    __syncthreads();

    // 32 threads
    if (threadIdx.x < 16)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 16].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 16].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 16].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 16].j;
        }

    __syncthreads();

    // 16 threads
    if (threadIdx.x < 8)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 8].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 8].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 8].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 8].j;
        }

    __syncthreads();

    // 8 threads
    if (threadIdx.x < 4)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 4].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 4].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 4].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 4].j;
        }

    __syncthreads();

    // 4 threads
    if (threadIdx.x < 2)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 2].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 2].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 2].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 2].j;
        }

    __syncthreads();

    // 2 threads
    if (threadIdx.x < 1)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 1].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 1].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 1].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 1].j;
        }

    __syncthreads();

    // 1 thread left
    // 1st thread in a block holds the best result
    if (threadIdx.x == 0) {
#ifndef REDUCE_ON_CPU
        // reduction on GPU

        if (best_values[threadIdx.x].minchange < best_2opt->minchange) {
            // inter-block reduction
            atomicMin (& (best_2opt->minchange), best_values[threadIdx.x].minchange);
            // synchronize
            lock (mutex);

            if (best_values[threadIdx.x].minchange == best_2opt->minchange) {
                best_2opt->i = best_values[threadIdx.x].i;
                best_2opt->j = best_values[threadIdx.x].j;
            }

            unlock (mutex);
        }

#else
        // reduction on host
        best_2opt[blockIdx.x] = best_values[threadIdx.x];
#endif
    }
}

// currently this code is hard-coded for 16x1024 thread configuration
// the subproblem block size is 2048x2048 with 128x128 thread groups iterating 16x16
// times (16 times each direction)
#ifndef USE_ORDERED_COORDS
__launch_bounds__ (1024, 64)
__global__ void kernel2opt_extended (city_coords * coords_global,
                                     ROUTE_DATA_TYPE * device_route, int _cities,
                                     int *mutex, struct best2_out *best_2opt) {
#else
__launch_bounds__ (1024, 64)
__global__ void kernel2opt_extended (city_coords * coords_global, int _cities,
                                     int *mutex, struct best2_out *best_2opt) {
#endif
    register int    local_id = threadIdx.x + blockIdx.x * blockDim.x;
    register int    i,
             j;
    register int    change;
    struct best2_out best;
    register int    rA_start,
             rB_start,
             rA_end,
             rB_end;
    // 2 ranges of coordinates
#ifndef LEGACY_GPUS
    __shared__ city_coords coords_extended_A[CUDA_MAX_COORDS_EXTENDED + 1];
    __shared__ city_coords coords_extended_B[CUDA_MAX_COORDS_EXTENDED + 1];
#else
    __shared__ city_coords coords_extended_A[CUDA_MAX_COORDS_EXTENDED_LEGACY + 1];
    __shared__ city_coords coords_extended_B[CUDA_MAX_COORDS_EXTENDED_LEGACY + 1];
#endif
    __shared__ int  cities;
#ifdef LEGACY_GPUS
    __shared__ best2_out best_values[256];
#else
    __shared__ best2_out best_values[1024];
#endif
    cities = _cities;
#ifdef LEGACY_GPUS
#define OFFSET 512
#else
#define OFFSET 2048
#endif
    register int    const1 = cities - OFFSET;
#ifdef K20
    register int    const2 = (cities - (cities / OFFSET) * OFFSET) / 256 + 1;
#else
    register int    const2 = (cities - (cities / OFFSET) * OFFSET) / 128 + 1;
#endif
#ifdef LEGACY_GPUS
    register int    iters_inside_a_blockB = 4;
#else
#ifndef K20
    register int    iters_inside_a_blockB = 16;
#else
    register int    iters_inside_a_blockB = 8;
#endif
#endif
    best.minchange = 99999;
    best.i = 1;
    best.j = 1;
    __syncthreads();

    // iterating over the subproblems defined by the coordinates'
    // ranges
    //
    // -|\
    // -|-\
    // -|--\
    // -|---\ i.e. A = (0,x), B = (y>0, z<size)
    // B|----\
    // -|-----\
    // -|------\
    // -+-------\
    // -> A

    for (register int a = 1; a < cities; a += OFFSET) {
        // if (a > cities - 2048)
        // a = cities - 2048;
        a = min (a, const1);
        rA_start = a - 1;
        rA_end = a + OFFSET;	// Vertical size
#ifndef USE_ORDERED_COORDS

        // copy the first range of coordinates
        for (register int k = rA_start + threadIdx.x; k < rA_end; k += blockDim.x) {
            coords_extended_A[k - rA_start] = coords_global[device_route[k]];
        }

#else

        for (register int k = rA_start + threadIdx.x; k < rA_end; k += blockDim.x) {
            coords_extended_A[k - rA_start] = coords_global[k];
        }

#endif

        for (register int b = const1; b > a - OFFSET; b -= OFFSET) {
            if ( (b < a) || (a == const1) ) {
                b = a;
            }

            rB_start = b - 1;
            rB_end = b + OFFSET;
#ifndef USE_ORDERED_COORDS

            // copy the second range of coordinates
            for (register int k = rB_start + threadIdx.x; k < rB_end;
                    k += blockDim.x) {
                coords_extended_B[k - rB_start] = coords_global[device_route[k]];
            }

#else

            for (register int k = rB_start + threadIdx.x; k < rB_end;
                    k += blockDim.x) {
                coords_extended_B[k - rB_start] = coords_global[k];
            }

#endif
            __syncthreads();
#ifdef LEGACY_GPUS
            iters_inside_a_blockB = 4;
#else
#ifndef K20
            iters_inside_a_blockB = 16;
#else
            iters_inside_a_blockB = 8;
#endif
#endif

            // check if there are some subblocks that can be skipped
            if (b == a)
                if (a != const1) {
                    iters_inside_a_blockB = const2;
                }

            // #if __CUDA_ARCH__ >= 300
#ifndef LEGACY_GPUS
#ifdef K20
#pragma unroll 8
#else
#pragma unroll 16
#endif
#endif

            // #endif
            // for a 2048x2048 problem size, each thread has to work 256 times...
            // (16x16)
            // since there are 16K threads running and 4,194,304 exchanges to be
            // checked
            for (register int re_i = 0; re_i < iters_inside_a_blockB; re_i++) {
                // #if __CUDA_ARCH__ >= 300
#ifndef LEGACY_GPUS
#ifdef K20
#pragma unroll 8
#else
#pragma unroll 16
#endif
#endif
                // #elif __CUDA_ARCH__ >= 210
                // seems to work with Fermi
                // #pragma unroll 4
                // #endif
#ifndef LEGACY_GPUS
#ifdef K20

                for (register int re_j = 0; re_j < 8; re_j++) {
#else

                for (register int re_j = 0; re_j < 16; re_j++) {
#endif
#else

                for (register int re_j = 0; re_j < 4; re_j++) {
#endif
#ifdef K20
                    // 256x256 block, if different this has to be changed
                    i = (local_id >> 8) + (re_i << 8);
                    j = mod256 (local_id) + (re_j << 8);
#else
                    // 128x128 block, if different this has to be changed
                    i = (local_id >> 7) + (re_i << 7);
                    j = mod128 (local_id) + (re_j << 7);
#endif

                    // only triangular matrix
                    if (i + b > j + a + 1) {
                        // calculate the effect of (i,j) swap
                        change =
                            calculateDistance2D_extended (j + 1, i + 1,
                                                          coords_extended_A,
                                                          coords_extended_B) +
                            calculateDistance2D_extended (i, j, coords_extended_B,
                                                          coords_extended_A) -
                            calculateDistance2D_extended (i, i + 1, coords_extended_B,
                                                          coords_extended_B) -
                            calculateDistance2D_extended (j, j + 1, coords_extended_A,
                                                          coords_extended_A);

                        if (change < best.minchange) {
                            best.minchange = change;
                            best.i = i + rB_start + 1;
                            best.j = j + rA_start + 1;
                        }
                    }
                }
            }

            __syncthreads();
        }
    }

    // intra-block reduction...it doesn't look pretty, but works
    best_values[threadIdx.x] = best;
    __syncthreads();
#ifndef LEGACY_GPUS

    // 1024 threads
    if (threadIdx.x < 512)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 512].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 512].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 512].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 512].j;
        }

    __syncthreads();

    // 512 threads
    if (threadIdx.x < 256)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 256].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 256].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 256].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 256].j;
        }

    __syncthreads();
#endif

    // 256 threads
    if (threadIdx.x < 128)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 128].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 128].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 128].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 128].j;
        }

    __syncthreads();

    // 128 threads
    if (threadIdx.x < 64)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 64].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 64].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 64].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 64].j;
        }

    __syncthreads();

    // 64 threads
    if (threadIdx.x < 32)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 32].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 32].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 32].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 32].j;
        }

    __syncthreads();

    // 32 threads
    if (threadIdx.x < 16)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 16].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 16].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 16].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 16].j;
        }

    __syncthreads();

    // 16 threads
    if (threadIdx.x < 8)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 8].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 8].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 8].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 8].j;
        }

    __syncthreads();

    // 8 threads
    if (threadIdx.x < 4)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 4].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 4].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 4].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 4].j;
        }

    __syncthreads();

    // 4 threads
    if (threadIdx.x < 2)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 2].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 2].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 2].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 2].j;
        }

    __syncthreads();

    // 2 threads
    if (threadIdx.x < 1)
        if (best_values[threadIdx.x].minchange >
                best_values[threadIdx.x + 1].minchange) {
            best_values[threadIdx.x].minchange =
                best_values[threadIdx.x + 1].minchange;
            best_values[threadIdx.x].i = best_values[threadIdx.x + 1].i;
            best_values[threadIdx.x].j = best_values[threadIdx.x + 1].j;
        }

    __syncthreads();

    // 1 thread left
    // 1st thread in a block holds the best result
    if (threadIdx.x == 0) {
#ifndef REDUCE_ON_CPU

        // reduction on GPU
        if (best_values[threadIdx.x].minchange < best_2opt->minchange) {
            // inter-block reduction
            atomicMin (& (best_2opt->minchange), best_values[threadIdx.x].minchange);
            // synchronize
            lock (mutex);

            if (best_values[threadIdx.x].minchange == best_2opt->minchange) {
                best_2opt->i = best_values[threadIdx.x].i;
                best_2opt->j = best_values[threadIdx.x].j;
            }

            unlock (mutex);
        }

#else
        // reduction on host
        best_2opt[blockIdx.x] = best_values[threadIdx.x];
#endif
    }
}
