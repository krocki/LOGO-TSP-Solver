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

#ifndef _DEFS_CUDA_
#define _DEFS_CUDA_

// maximum number of cities that can be used in the simple GPU 2-opt algorithm
// limited by the shared memory size
// shared memory needed = MAX_CITIES * sizeof(city_coords)
#define CUDA_MAX_SIZE_SIMPLE 4096
#define CUDA_MAX_SIZE_SIMPLE_LEGACY 1536

// maximum number of cities that can be used in the extended GPU 2-opt algorithm
// limited by the host and device memory size
// global memory needed = CUDA_MAX_SIZE_EXTENDED * sizeof(city_coords)
// shared memory needed -> set in the algorithm
#define CUDA_MAX_SIZE_EXTENDED MAX_CITIES


// maximum number of coordinates stored in the extended GPU 2-opt algorithm
// in the shared memory = 2*MAX_COORDS_EXTENDED
#define CUDA_MAX_COORDS_EXTENDED 2048
#define CUDA_MAX_COORDS_EXTENDED_LEGACY 512

// changed to #if __CUDA_ARCH__ >= 300
// Kepler GPU optimizations, loop unrolling
// if uncommented the code might not work properly on older devices
// #define COMPUTE_CAPABILITY_30

// little optimization (Fermi GPUs), comment if any problems still exist (i.e.
// kernel returns 0s always) or it won't run
// #define COMPUTE_CAPABILITY_21

#define DEFAULT_CUDA_DEVICE 0

// inter block reduction on host (to avoid atomics on GPU)
// #define REDUCE_ON_CPU

// max GPU blocks
#define MAX_BLOCKS_CUDA 1024

typedef struct {

    unsigned        id;
    string          device_string;
    unsigned        MPs;
    unsigned        coresPerMP;
    float           clockRate;
    unsigned long   global_mem_size;
    unsigned long   local_mem_size;

} cuda_dev_info;

#define K20

#endif
