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

#include <CUDASolver.h>
#include <UtilsCUDA.h>

#ifdef LEGACY_GPUS
#define REDUCE_ON_CPU
#endif

#ifdef HAVE_CUDA

#ifdef __NVCC__

#ifndef USE_ORDERED_COORDS
__global__ void kernel2opt (city_coords * coords_global, int cities,
                            ROUTE_DATA_TYPE * device_route, unsigned long counter,
                            unsigned int iter, int *mutex,
                            struct best2_out *best_2opt);
__global__ void kernel2opt_extended (city_coords * coords_global,
                                     ROUTE_DATA_TYPE * device_route, int cities,
                                     int *mutex, struct best2_out *best_2opt);
#else
__global__ void kernel2opt (city_coords * coords_global, int cities,
                            unsigned long counter, unsigned int iter, int *mutex,
                            struct best2_out *best_2opt);
__global__ void kernel2opt_extended (city_coords * coords_global, int cities,
                                     int *mutex, struct best2_out *best_2opt);
#endif

#endif


void
CUDASolver::init() {
}

void
CUDASolver::init (int d) {
    dev = d;
    cudaSetDevice (dev);
    cudaMallocHost (&host_coords, sizeof (city_coords) * CUDA_MAX_SIZE_EXTENDED);

    if (host_coords == NULL) {
        fprintf (stderr,
                 "initCUDA:host_coords:Could not allocate that much memory (%ld B)",
                 sizeof (city_coords) * CUDA_MAX_SIZE_EXTENDED);
        exit (-1);
    }

#ifdef USE_ORDERED_COORDS
    cudaMallocHost (&host_coords_ordered,
                    sizeof (city_coords) * CUDA_MAX_SIZE_EXTENDED);

    if (host_coords_ordered == NULL) {
        fprintf (stderr,
                 "initCUDA:host_coords_ordered:Could not allocate that much memory (%ld B)",
                 sizeof (city_coords) * CUDA_MAX_SIZE_EXTENDED);
        exit (-1);
    }

#endif
    UtilsCUDA::CudaTest ("CUDA init: cudaMallocHost");
    memcpy (host_coords, coords, sizeof (struct cc) * size);
    trace ("Allocating CUDA memory...\n");

    if (cudaSuccess !=
            cudaMalloc ( (void **) &device_coords, sizeof (struct cc) * size) ) {
        fprintf (stderr, "cudaMalloc - cannot allocate memory\n");
    }

    UtilsCUDA::
    CudaTest ("cudaMalloc((void **)&device_coords, sizeof(struct cc) * size)");

    if (cudaSuccess !=
            cudaMalloc ( (void **) &device_route, sizeof (ROUTE_DATA_TYPE) * size) ) {
        fprintf (stderr, "cudaMalloc - cannot allocate memory\n");
    }

    UtilsCUDA::
    CudaTest
    ("cudaMalloc((void **)&device_route, sizeof(ROUTE_DATA_TYPE) * size)");
#ifndef REDUCE_ON_CPU

    if (cudaSuccess !=
            cudaMalloc ( (void **) &device_out_2opt, sizeof (struct best2_out) ) )
#else
    if (cudaSuccess !=
            cudaMalloc ( (void **) &device_out_2opt,
                         sizeof (struct best2_out) * MAX_BLOCKS_CUDA) )
#endif
        fprintf (stderr, "cudaMalloc - cannot allocate memory\n");

    UtilsCUDA::
    CudaTest ("cudaMalloc((void **)&device_out_2opt, sizeof(struct best2_out))");
    trace ("Copying coordinates to CUDA memory...\n");

    if (cudaSuccess !=
            cudaMemcpy (device_coords, host_coords, sizeof (struct cc) * size,
                        cudaMemcpyHostToDevice) ) {
        fprintf (stderr, "Copying of coordinates to device failed\n");
    }

    UtilsCUDA::
    CudaTest
    ("cudaMemcpy(device_coords, host_coords,  sizeof(struct cc) * size, cudaMemcpyHostToDevice)");
    trace ("Done.\n");
#ifdef K20
    gpuThreads = 1024;
    gpuBlocks = 64;
#endif
    // simple kernel
    number_of_2opts = (long) (size - 3) * (long) (size - 2) / 2;
    iterations = (number_of_2opts / (gpuThreads * gpuBlocks) ) + 1;
#ifndef REDUCE_ON_CPU
    dev_mutex = 0;
    cudaMalloc (&dev_mutex, sizeof (int) );
#endif
    zero.minchange = 0;
    zero.i = 0;
    zero.j = 0;
    initialized = 1;
    vector < cuda_dev_info > info = UtilsCUDA::getCUDAInfo (-1, 0);
    description += info[dev].device_string;
}

void
CUDASolver::close() {
    cleanCUDA();
};

struct process_time
CUDASolver::benchmark (vector < ROUTE_DATA_TYPE > &route) {

    struct timeval  start,
            end;
    process_time    time;

#ifndef REDUCE_ON_CPU
    struct best2_out out;
#else
    struct best2_out out[MAX_BLOCKS_CUDA];
#endif

    cudaSetDevice (dev);

#ifndef REDUCE_ON_CPU
    // reset mutex, just in case
    cudaMemset (dev_mutex, 0, sizeof (int) );
#endif

    cudaThreadSynchronize();
    gettimeofday (&start, NULL);

    // copying HtoD
#ifndef USE_ORDERED_COORDS

    // copy the route
    if (cudaSuccess !=
            cudaMemcpy (device_route, &route[0], sizeof (ROUTE_DATA_TYPE) * size,
                        cudaMemcpyHostToDevice) ) {
        fprintf (stderr, "tour copy to device failed\n");
    }

    UtilsCUDA::
    CudaTest
    ("cudaMemcpy(device_route, &route[0], sizeof(ROUTE_DATA_TYPE) * size, cudaMemcpyHostToDevice)");
#else

    // reorder the route to allow sequential CUDA access
    for (ROUTE_DATA_TYPE i = 0; i < size; i++) {
        host_coords_ordered[i] = host_coords[route[i]];
    }

    if (cudaSuccess !=
            cudaMemcpy (device_coords, host_coords_ordered, sizeof (struct cc) * size,
                        cudaMemcpyHostToDevice) ) {
        fprintf (stderr, "Copying of coordinates to device failed\n");
    }

    UtilsCUDA::
    CudaTest
    ("cudaSuccess != cudaMemcpy(device_coords, host_coords_ordered,  sizeof(struct cc) * size, cudaMemcpyHostToDevice)");

#endif

    // reset ids of edges selected to swap
    if (cudaSuccess !=
            cudaMemcpy (device_out_2opt, &zero, sizeof (struct best2_out),
                        cudaMemcpyHostToDevice) ) {
        fprintf (stderr, "copying of minchange to device failed\n");
    }

    UtilsCUDA::
    CudaTest
    ("cudaMemcpy(device_out_2opt, &zero, sizeof(struct best2_out), cudaMemcpyHostToDevice)");

    cudaThreadSynchronize();
    gettimeofday (&end, NULL);

    time.HtD_time = getTimeDiff (&start, &end);

    cudaThreadSynchronize();
    gettimeofday (&start, NULL);

    // kernel execution
    executeKernel();

    cudaThreadSynchronize();
    gettimeofday (&end, NULL);

    time.kernel_time = getTimeDiff (&start, &end);

    cudaThreadSynchronize();
    gettimeofday (&start, NULL);

    // copying DtoH
#ifndef REDUCE_ON_CPU

    if (cudaSuccess !=
            cudaMemcpy (&out, device_out_2opt, sizeof (struct best2_out),
                        cudaMemcpyDeviceToHost) )
#else
    if (cudaSuccess !=
            cudaMemcpy (&out, device_out_2opt, sizeof (struct best2_out) * gpuBlocks,
                        cudaMemcpyDeviceToHost) )
#endif
        fprintf (stderr, "copying of struct best2_out from device failed\n");

    UtilsCUDA::
    CudaTest
    ("cudaMemcpy(&out, device_out_2opt, sizeof(struct best2_out), cudaMemcpyDeviceToHost)");

#ifdef REDUCE_ON_CPU
    // inter-block reduce (to avoid atomics)
    int             best_change = out[0].minchange;
    int             best_i = out[0].i;
    int             best_j = out[0].j;

    for (unsigned long i = 1; i < gpuBlocks; i++) {
        if (out[i].minchange < best_change) {
            best_change = out[i].minchange;
            best_i = out[i].i;
            best_j = out[i].j;
        }
    }

    out[0].minchange = best_change;
    out[0].i = best_i;
    out[0].j = best_j;
#endif

    cudaThreadSynchronize();
    gettimeofday (&end, NULL);

    time.DtH_time = getTimeDiff (&start, &end);

#ifndef REDUCE_ON_CPU
    // debug code
    trace ("[%s] CUDA Local Optimization: 2-opt pair found (%d,%d) -> change %d\n",
           description.c_str(), out.i, out.j, out.minchange);
#else
    trace ("[%s] CUDA Local Optimization: 2-opt pair found (%d,%d) -> change %d\n",
           description.c_str(), out[0].i, out[0].j, out[0].minchange);

#endif
    return time;

}

struct best2_out
CUDASolver::optimizeStep (const vector < ROUTE_DATA_TYPE > &route) {

#ifndef REDUCE_ON_CPU
    struct best2_out out;
#else
    struct best2_out out[MAX_BLOCKS_CUDA];
#endif

    cudaSetDevice (dev);

#ifndef REDUCE_ON_CPU
    // reset mutex, just in case
    cudaMemset (dev_mutex, 0, sizeof (int) );
#endif

    // copying HtoD
#ifndef USE_ORDERED_COORDS

    // copy the route
    if (cudaSuccess !=
            cudaMemcpy (device_route, &route[0], sizeof (ROUTE_DATA_TYPE) * size,
                        cudaMemcpyHostToDevice) ) {
        fprintf (stderr, "tour copy to device failed\n");
    }

    UtilsCUDA::
    CudaTest
    ("cudaMemcpy(device_route, &route[0], sizeof(ROUTE_DATA_TYPE) * size, cudaMemcpyHostToDevice)");
#else

    // reorder the route to allow sequential CUDA access
    for (ROUTE_DATA_TYPE i = 0; i < size; i++) {
        host_coords_ordered[i] = host_coords[route[i]];
    }

    if (cudaSuccess !=
            cudaMemcpy (device_coords, host_coords_ordered, sizeof (struct cc) * size,
                        cudaMemcpyHostToDevice) ) {
        fprintf (stderr, "Copying of coordinates to device failed\n");
    }

    UtilsCUDA::
    CudaTest
    ("cudaSuccess != cudaMemcpy(device_coords, host_coords_ordered,  sizeof(struct cc) * size, cudaMemcpyHostToDevice)");

#endif

    // reset ids of edges selected to swap
    if (cudaSuccess !=
            cudaMemcpy (device_out_2opt, &zero, sizeof (struct best2_out),
                        cudaMemcpyHostToDevice) ) {
        fprintf (stderr, "copying of minchange to device failed\n");
    }

    UtilsCUDA::
    CudaTest
    ("cudaMemcpy(device_out_2opt, &zero, sizeof(struct best2_out), cudaMemcpyHostToDevice)");

    // kernel execution
    executeKernel();

    // get ids of edges selected to swap
#ifndef REDUCE_ON_CPU

    if (cudaSuccess !=
            cudaMemcpy (&out, device_out_2opt, sizeof (struct best2_out),
                        cudaMemcpyDeviceToHost) )
#else
    if (cudaSuccess !=
            cudaMemcpy (&out, device_out_2opt, sizeof (struct best2_out) * gpuBlocks,
                        cudaMemcpyDeviceToHost) )
#endif
        fprintf (stderr, "copying of struct best2_out from device failed\n");

    UtilsCUDA::
    CudaTest
    ("cudaMemcpy(&out, device_out_2opt, sizeof(struct best2_out), cudaMemcpyDeviceToHost)");

#ifdef REDUCE_ON_CPU
    // inter-block reduce (to avoid atomics)
    int             best_change = out[0].minchange;
    int             best_i = out[0].i;
    int             best_j = out[0].j;

    for (unsigned long i = 1; i < gpuBlocks; i++) {
        if (out[i].minchange < best_change) {
            best_change = out[i].minchange;
            best_i = out[i].i;
            best_j = out[i].j;
        }
    }

    out[0].minchange = best_change;
    out[0].i = best_i;
    out[0].j = best_j;
    return out[0];
#else
    return out;
#endif

}

void
CUDASolver::cleanCUDA (void) {
    cudaSetDevice (dev);
    trace ("Deallocating CUDA memory...\n");
    cudaFree (device_coords);
    cudaFree (device_route);
    cudaFree (device_out_2opt);
    cudaFreeHost (host_coords);
    cudaFree (dev_mutex);
#ifdef USE_ORDERED_COORDS
    cudaFreeHost (host_coords_ordered);
#endif
    trace ("Done.\n");
}

void
CUDASolver::executeKernel() {
#ifndef LEGACY_GPUS

    if (size < CUDA_MAX_SIZE_SIMPLE) {
#else

    if (size < CUDA_MAX_SIZE_SIMPLE_LEGACY) {
#endif
#ifndef USE_ORDERED_COORDS
        kernel2opt <<< gpuBlocks, gpuThreads >>> (device_coords, size, device_route,
                number_of_2opts, iterations,
                dev_mutex, device_out_2opt);
#else
        kernel2opt <<< gpuBlocks, gpuThreads >>> (device_coords, size,
                number_of_2opts, iterations,
                dev_mutex, device_out_2opt);
#endif

    } else if (size < CUDA_MAX_SIZE_EXTENDED) {
#ifndef USE_ORDERED_COORDS
        kernel2opt_extended <<< gpuBlocks, gpuThreads >>> (device_coords,
                device_route, size,
                dev_mutex,
                device_out_2opt);
#else
        kernel2opt_extended <<< gpuBlocks, gpuThreads >>> (device_coords, size,
                dev_mutex,
                device_out_2opt);
#endif
    }
}

#endif
