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

#include <twoOptLocalSolver.h>

#ifdef __NVCC__
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_defs.h>
#endif


#ifndef _CUDA_SOLVER_
#define _CUDA_SOLVER_


#ifdef HAVE_CUDA

class           CUDASolver: public TwoOptLocalSolver {

public:

    CUDASolver (city_coords * _coords, cmdArguments * _args, string _description,
                short _deviceId, ROUTE_DATA_TYPE _size) : TwoOptLocalSolver (_coords,
                            _args,
                            _description,
                            _deviceId,
                            _size) {
#ifdef LEGACY_GPUS
        setGpuBlocks (64);
        setGpuThreads (256);
#else
        setGpuBlocks (DEFAULT_GPU_BLOCKS);
        setGpuThreads (DEFAULT_GPU_THREADS);
#endif
        init (_deviceId);
    };

    CUDASolver (city_coords * _coords, cmdArguments * _args, string _description,
                short _deviceId, ROUTE_DATA_TYPE _size,
                unsigned _tID) : TwoOptLocalSolver (_coords, _args, _description,
                            _deviceId, _size, _tID) {
#ifdef LEGACY_GPUS
        setGpuBlocks (64);
        setGpuThreads (256);
#else
        setGpuBlocks (DEFAULT_GPU_BLOCKS);
        setGpuThreads (DEFAULT_GPU_THREADS);
#endif
        init (_deviceId);
    };

    // + blocks/threads
    CUDASolver (city_coords * _coords, cmdArguments * _args, string _description,
                short _deviceId, ROUTE_DATA_TYPE _size, unsigned _tID,
                unsigned _gpublocks, unsigned _gputhreads) : TwoOptLocalSolver (_coords,
                            _args,
                            _description,
                            _deviceId,
                            _size,
                            _tID) {
        setGpuBlocks (_gpublocks);
        setGpuThreads (_gputhreads);
        init (_deviceId);
    };

    ~CUDASolver() {
        if (initialized) {
            close();
        }
    };

    void            setGpuBlocks (unsigned b) {
        gpuBlocks = b;
    };
    void            setGpuThreads (unsigned t) {
        gpuThreads = t;
    };

protected:

    struct process_time benchmark (vector < ROUTE_DATA_TYPE > &route);
    struct best2_out optimizeStep (const vector < ROUTE_DATA_TYPE > &route);

    void            init (void);
    void            init (int d);
    void            close (void);

    void            cleanCUDA (void);

    city_coords    *device_coords;
    city_coords    *host_coords;
    ROUTE_DATA_TYPE *device_route;
    struct best2_out *device_out_2opt;
    city_coords    *host_coords_ordered;
    short           dev;

    void            executeKernel (void);

    unsigned        gpuBlocks;
    unsigned        gpuThreads;

    // simple kernel
    unsigned long long number_of_2opts;
    unsigned int    iterations;

    struct best2_out zero;
    int            *dev_mutex;

    unsigned        initialized;
};


#endif
#endif
