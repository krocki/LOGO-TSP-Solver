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
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <cl_defs.h>

#ifndef _CL_SOLVER_
#define _CL_SOLVER_

#ifdef HAVE_OPENCL


class           CLSolver: public TwoOptLocalSolver {

public:

    CLSolver (city_coords * _coords, cmdArguments * _args, string _description,
              long _deviceId, ROUTE_DATA_TYPE _size) : TwoOptLocalSolver (_coords, _args,
                          _description,
                          _deviceId,
                          _size) {
        init ( (cl_device_id) _deviceId);
    };

    CLSolver (city_coords * _coords, cmdArguments * _args, string _description,
              long _deviceId, ROUTE_DATA_TYPE _size,
              unsigned _tID) : TwoOptLocalSolver (_coords, _args, _description,
                          _deviceId, _size, _tID) {
        init ( (cl_device_id) _deviceId);
    };

    ~CLSolver() {
        if (initialized) {
            close();
        }
    };

    vector < deviceInfo > listDevices();

protected:

    void            init();
    void            init (cl_device_id dev);
    void            close (void);

    void            executeKernel (int wait = 0);

    struct process_time benchmark (vector < ROUTE_DATA_TYPE > &route);
    struct best2_out optimizeStep (const vector < ROUTE_DATA_TYPE > &route);

    void            chooseCLDevice (cl_dev_info d_num);

    void            cleanCL (void);
    void            initCL (cmdArguments * args, city_coords * coords,
                            ROUTE_DATA_TYPE c, int profiling, cl_device_id dev);
    void            saveBinary (cl_program program, cl_device_id device, char *name);
    double          executionTime (cl_event gpuExecution);
    cl_program      compileProgram (const char *const header_file,
                                    const char *const kernel_file,
                                    cl_context cxContext, cl_device_id d);
    char           *readFile (const char *filename, size_t * length);
    void            copyFromDevice (const cl_mem dMem, void *const hostPtr,
                                    const unsigned size,
                                    cl_command_queue commandQueue,
                                    cl_event * gpuDone);
    void            runKernel (size_t localWorkSize, size_t globalWorkSize,
                               cl_device_id device, cl_command_queue commandQueue,
                               cl_kernel kernel, cl_event * gpuExecution);

    const char     *oclErrorString (cl_int error);
    void            checkError (const cl_int ciErrNum, const char *const operation);

    cl_context      context;
    cl_program      program;
    cl_program      program_e;

    cl_command_queue commandQueue;

    cl_int          ciErrNum,
                    ret;
    cl_device_id    d;

    city_coords    *host_coords_cl;
    city_coords    *host_coords_ordered_cl;

    cl_mem          host_coords_ordered_pinned_cl;

    cl_mem          device_coords_ordered_cl;
    cl_mem          device_out_2opt_cl;

    cl_mem          host_out_2opt_pinned_cl;
    struct best2_out *host_out_2opt_cl;

    ROUTE_DATA_TYPE cities_cl;

    cl_kernel       kernel_2opt;
    cl_kernel       kernel_2opt_e;

    ROUTE_DATA_TYPE CL_MAX_SIZE_SIMPLE;
    ROUTE_DATA_TYPE CL_MAX_SIZE_EXTENDED;
    unsigned        LOCAL_WORK_SIZE;
    unsigned        BLOCKS;

    // simple kernel
    size_t          globalWorkSize;
    unsigned long long number_of_2opts;
    unsigned int    iterations;

    // extended kernel
    size_t          localWorkSize;
    size_t          blocks;
    size_t          itersInside;

    int             initialized;
};

#endif
#endif
