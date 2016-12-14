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

#include <CLSolver.h>
#include <UtilsCL.h>

#ifdef HAVE_OPENCL

void
CLSolver::init() {
}

void
CLSolver::init (cl_device_id dev) {
    if (dev == NULL) {
        trace ("CL Solver - Wrong device\n");
        exit (-1);
    }

    context = 0;
    program = 0;
    program_e = 0;
    initCL (args, coords, size, 0, dev);
    localWorkSize = LOCAL_WORK_SIZE;
    blocks = BLOCKS;

    if (size > CL_MAX_CITIES_SIMPLE) {
        if (CL_MAX_SIZE_EXTENDED < 2048) {
            localWorkSize = 256;
            itersInside = 14;
            blocks = 64;

        } else {
            localWorkSize = 1024;
            itersInside = 16;
            blocks = 16;
        }
    }

#ifdef LEGACY_GPUS
    localWorkSize = 256;
    blocks = 64;
    itersInside = 14;
#endif

    if (blocks > MAX_BLOCKS) {
        printf ("Too many blocks: %u (max %u)\n", (unsigned) blocks,
                (unsigned) MAX_BLOCKS);
    }

    globalWorkSize = blocks * localWorkSize;
    number_of_2opts = (long) (size - 3) * (long) (size - 2) / 2;
    iterations = (number_of_2opts / (globalWorkSize) ) + 1;
    initialized = 1;
};

void
CLSolver::close() {
    trace ("CL Solver - Clean Up\n");
    cleanCL();
};

struct process_time
CLSolver::benchmark (vector < ROUTE_DATA_TYPE > &route) {

    struct best2_out out;
    struct timeval  start,
            end;
    process_time    time;

    int             best_change = INT_MAX;

    // reordering coordinates
    for (ROUTE_DATA_TYPE i = 0; i < size; i++) {
        host_coords_ordered_cl[i] = host_coords_cl[route[i]];
    }

    cl_event        copy_event;

    gettimeofday (&start, NULL);

    // copying HtoD
    CL_SAFE_CALL (clEnqueueWriteBuffer
                  (commandQueue, device_coords_ordered_cl, CL_TRUE, 0,
                   sizeof (city_coords) * size, host_coords_ordered_cl, 0, NULL,
                   &copy_event) );

    clWaitForEvents (1, &copy_event);

    gettimeofday (&end, NULL);

    time.HtD_time = getTimeDiff (&start, &end);

    gettimeofday (&start, NULL);

    // kernel execution
    executeKernel (1);

    gettimeofday (&end, NULL);

    time.kernel_time = getTimeDiff (&start, &end);

    gettimeofday (&start, NULL);

    // copying DtoH
    CL_SAFE_CALL (clEnqueueReadBuffer
                  (commandQueue, device_out_2opt_cl, CL_TRUE, 0,
                   sizeof (struct best2_out) * blocks, host_out_2opt_cl, 0, NULL,
                   &copy_event) );

    gettimeofday (&end, NULL);

    time.DtH_time = getTimeDiff (&start, &end);
    // inter-block reduce (to avoid atomics)

    best_change = host_out_2opt_cl[0].minchange;
    int             best_i = host_out_2opt_cl[0].i;
    int             best_j = host_out_2opt_cl[0].j;

    for (unsigned long i = 1; i < blocks; i++) {
        if (host_out_2opt_cl[i].minchange < best_change) {
            best_change = host_out_2opt_cl[i].minchange;
            best_i = host_out_2opt_cl[i].i;
            best_j = host_out_2opt_cl[i].j;
        }
    }

    out.minchange = best_change;
    out.i = best_i;
    out.j = best_j;

    // debug code
    trace ("[%s] OpenCL Local Optimization: 2-opt pair found (%d,%d) -> change %d\n",
           description.c_str(), out.i, out.j, out.minchange);

    return time;

};

// Implementation of the 2-opt exchange step

void
CLSolver::executeKernel (int wait) {
    cl_event        execution_event;

    if (size < CL_MAX_CITIES_SIMPLE) {
        // processing
        clSetKernelArg (kernel_2opt, 0, sizeof (cl_mem),
                        (void *) &device_coords_ordered_cl);
        clSetKernelArg (kernel_2opt, 1, sizeof (cl_mem), (void *) &device_out_2opt_cl);
        clSetKernelArg (kernel_2opt, 2, sizeof (city_coords) * CL_MAX_SIZE_SIMPLE,
                        NULL);
        clSetKernelArg (kernel_2opt, 3, sizeof (struct best2_out) * localWorkSize,
                        NULL);
        clSetKernelArg (kernel_2opt, 4, sizeof (unsigned int),
                        (void *) &localWorkSize);
        clSetKernelArg (kernel_2opt, 5, sizeof (ROUTE_DATA_TYPE), (void *) &size);
        clSetKernelArg (kernel_2opt, 6, sizeof (unsigned long),
                        (void *) &number_of_2opts);
        clSetKernelArg (kernel_2opt, 7, sizeof (unsigned int), (void *) &iterations);
        ciErrNum =
            clEnqueueNDRangeKernel (commandQueue, kernel_2opt, 1, NULL,
                                    &globalWorkSize, &localWorkSize, 0, NULL,
                                    &execution_event);

    } else if (size < CL_MAX_CITIES) {
        // processing
        clSetKernelArg (kernel_2opt_e, 0, sizeof (cl_mem),
                        (void *) &device_coords_ordered_cl);
        clSetKernelArg (kernel_2opt_e, 1, sizeof (cl_mem),
                        (void *) &device_out_2opt_cl);
        clSetKernelArg (kernel_2opt_e, 2,
                        sizeof (city_coords) * (CL_MAX_SIZE_EXTENDED + 1), NULL);
        clSetKernelArg (kernel_2opt_e, 3,
                        sizeof (city_coords) * (CL_MAX_SIZE_EXTENDED + 1), NULL);
        clSetKernelArg (kernel_2opt_e, 4, sizeof (struct best2_out) * localWorkSize,
                        NULL);
        clSetKernelArg (kernel_2opt_e, 5, sizeof (int),
                        (void *) &CL_MAX_SIZE_EXTENDED);
        clSetKernelArg (kernel_2opt_e, 6, sizeof (int), (void *) &localWorkSize);
        clSetKernelArg (kernel_2opt_e, 7, sizeof (int), (void *) &itersInside);
        clSetKernelArg (kernel_2opt_e, 8, sizeof (ROUTE_DATA_TYPE), (void *) &size);
        ciErrNum =
            clEnqueueNDRangeKernel (commandQueue, kernel_2opt_e, 1, NULL,
                                    &globalWorkSize, &localWorkSize, 0, NULL,
                                    &execution_event);
    }

    if (wait) {
        clWaitForEvents (1, &execution_event);
    }

    UtilsCL::checkError (ciErrNum, "clEnqueueNDRangeKernel");
}

struct best2_out
CLSolver::optimizeStep (const vector < ROUTE_DATA_TYPE > &route) {

    struct best2_out out;
    out.minchange = 0;
    int             best_change = INT_MAX;

    // reordering coordinates
    for (ROUTE_DATA_TYPE i = 0; i < size; i++) {
        host_coords_ordered_cl[i] = host_coords_cl[route[i]];
    }

    // copying HtoD
    CL_SAFE_CALL (clEnqueueWriteBuffer
                  (commandQueue, device_coords_ordered_cl, CL_TRUE, 0,
                   sizeof (city_coords) * size, host_coords_ordered_cl, 0, NULL,
                   NULL) );

    // kernel execution
    executeKernel();

    // copying DtoH
    CL_SAFE_CALL (clEnqueueReadBuffer
                  (commandQueue, device_out_2opt_cl, CL_TRUE, 0,
                   sizeof (struct best2_out) * blocks, host_out_2opt_cl, 0, NULL,
                   NULL) );


    // inter-block reduce (to avoid atomics)
    best_change = host_out_2opt_cl[0].minchange;
    int             best_i = host_out_2opt_cl[0].i;
    int             best_j = host_out_2opt_cl[0].j;

    for (unsigned long i = 1; i < blocks; i++) {
        if (host_out_2opt_cl[i].minchange < best_change) {
            best_change = host_out_2opt_cl[i].minchange;
            best_i = host_out_2opt_cl[i].i;
            best_j = host_out_2opt_cl[i].j;
        }
    }

    out.minchange = best_change;
    out.i = best_i;
    out.j = best_j;

    return out;
};

void
CLSolver::cleanCL (void) {
    trace ("Releasing OpenCL resources...\n");
    // release context

    if (context) {
        CL_SAFE_CALL (clReleaseContext (context) );
        context = 0;
    }

    // release program
    if (program) {
        CL_SAFE_CALL (clReleaseProgram (program) );
        program = 0;
    }

    if (program_e) {
        CL_SAFE_CALL (clReleaseProgram (program_e) );
        program_e = 0;
    }

    if (kernel_2opt) {
        CL_SAFE_CALL (clReleaseKernel (kernel_2opt) );
    }

    clEnqueueUnmapMemObject (commandQueue, host_coords_ordered_pinned_cl,
                             (void *) host_coords_ordered_cl, 0, NULL, NULL);
    clEnqueueUnmapMemObject (commandQueue, host_out_2opt_pinned_cl,
                             (void *) host_out_2opt_cl, 0, NULL, NULL);

    if (host_coords_ordered_pinned_cl) {
        clReleaseMemObject (host_coords_ordered_pinned_cl);
    }

    if (host_out_2opt_pinned_cl) {
        clReleaseMemObject (host_out_2opt_pinned_cl);
    }

    if (device_coords_ordered_cl) {
        clReleaseMemObject (device_coords_ordered_cl);
    }

    if (device_out_2opt_cl) {
        clReleaseMemObject (device_out_2opt_cl);
    }

    free (host_coords_cl);
    trace ("Done.\n");
}
void
CLSolver::initCL (cmdArguments * args, city_coords * coords, ROUTE_DATA_TYPE c,
                  int profiling, cl_device_id dev) {
    trace ("OpenCL init...\n");
    size = c;
    d = dev;
    trace ("Creating OpenCL context...\n");
    context = clCreateContext (0, 1, &d, NULL, NULL, &ciErrNum);
    UtilsCL::checkError (ciErrNum, "clCreateContext");

    if (profiling == 1) {
        commandQueue =
            clCreateCommandQueue (context, d, CL_QUEUE_PROFILING_ENABLE, &ciErrNum);

    } else {
        commandQueue = clCreateCommandQueue (context, d, 0, &ciErrNum);
    }

    UtilsCL::checkError (ciErrNum, "clCreateCommandQueue");
    trace ("Allocating host memory...\n");
    cl_mem_flags    inMemFlags = CL_MEM_READ_ONLY;
    cl_mem_flags    outMemFlags = CL_MEM_WRITE_ONLY;
    host_coords_cl = (city_coords *) ALLOC (sizeof (city_coords) * CL_MAX_CITIES);

    if (host_coords_cl == NULL) {
        fprintf (stderr,
                 "initCL:host_coords:Could not allocate that much memory (%ld B)",
                 sizeof (city_coords) * CL_MAX_CITIES);
        exit (-1);
    }

    memcpy (host_coords_cl, coords, sizeof (city_coords) * size);
    host_coords_ordered_pinned_cl =
        clCreateBuffer (context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                        sizeof (city_coords) * CL_MAX_CITIES, NULL, &ciErrNum);
    UtilsCL::checkError (ciErrNum,
                         "Allocating host memory : host_coords_ordered_pinned_cl");
    host_out_2opt_pinned_cl =
        clCreateBuffer (context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                        sizeof (struct best2_out) * MAX_BLOCKS, NULL, &ciErrNum);
    UtilsCL::checkError (ciErrNum,
                         "Allocating host memory : host_out_2opt_pinned_cl");
    trace ("Done.\n");
    trace ("Allocating device memory...\n");
    device_coords_ordered_cl =
        clCreateBuffer (context, inMemFlags, sizeof (city_coords) * CL_MAX_CITIES,
                        NULL, &ciErrNum);
    UtilsCL::checkError (ciErrNum,
                         "Allocating device memory; device_coords_ordered_cl");
    device_out_2opt_cl =
        clCreateBuffer (context, outMemFlags, sizeof (struct best2_out) * MAX_BLOCKS,
                        NULL, &ciErrNum);
    UtilsCL::checkError (ciErrNum, "Allocating device memory; device_out_2opt_cl");
    trace ("Done.\n");
    trace ("Mapping data...\n");
    host_coords_ordered_cl =
        (city_coords *) clEnqueueMapBuffer (commandQueue,
                                            host_coords_ordered_pinned_cl, CL_TRUE,
                                            CL_MAP_WRITE, 0,
                                            sizeof (city_coords) * CL_MAX_CITIES, 0,
                                            NULL, NULL, &ciErrNum);
    host_out_2opt_cl =
        (struct best2_out *) clEnqueueMapBuffer (commandQueue,
                host_out_2opt_pinned_cl, CL_TRUE,
                CL_MAP_READ, 0,
                sizeof (struct best2_out) *
                MAX_BLOCKS, 0, NULL, NULL,
                &ciErrNum);
    trace ("Done.\n");
    trace ("Compiling CL program...(./src/cl/opt.cl)\n");
    program = compileProgram ("", "./src/cl/opt.cl", context, d);
    UtilsCL::checkError (ciErrNum, "compileProgram");
    trace ("Done.\n");
    trace ("Creating 2-opt kernel...(cl2optKernel)\n");
    kernel_2opt = clCreateKernel (program, "cl2optKernel", &ciErrNum);
    UtilsCL::checkError (ciErrNum, "clCreateKernel");
    trace ("Done.\n");
    trace ("Compiling CL program...(./src/cl/opt_e.cl)\n");
    program_e = compileProgram ("", "./src/cl/opt_e.cl", context, d);
    trace ("Done.\n");
    trace ("Creating 2-opt kernel...(cl2optKernel_e)\n");
    kernel_2opt_e = clCreateKernel (program_e, "cl2optKernel_e", &ciErrNum);
    UtilsCL::checkError (ciErrNum, "clCreateKernel");
    trace ("Done.\n");
    cl_dev_info     dInfo = UtilsCL::getDevice (d);
    LOCAL_WORK_SIZE = dInfo.workgroup_size;
    CL_MAX_SIZE_SIMPLE =
        (dInfo.local_mem_size -
         LOCAL_WORK_SIZE * sizeof (struct best2_out) ) / sizeof (city_coords);
    CL_MAX_SIZE_EXTENDED =
        (dInfo.local_mem_size -
         LOCAL_WORK_SIZE * sizeof (struct best2_out) ) / (sizeof (city_coords) * 2);

    if (CL_MAX_SIZE_EXTENDED < 2048) {
        CL_MAX_SIZE_EXTENDED = 1792;

    } else {
        CL_MAX_SIZE_EXTENDED = 2048;
    }

    BLOCKS = 16 * 1024 / LOCAL_WORK_SIZE;
    description += dInfo.device_string;
    // temporary
    /*
     * LOCAL_WORK_SIZE = 256; BLOCKS = 64; CL_MAX_SIZE_EXTENDED = 1792;
     * CL_MAX_SIZE_SIMPLE = 3072;
     */
    // clInit, alloc host memory, device memory
}

void
CLSolver::saveBinary (cl_program program, cl_device_id device, char *name) {
    size_t          size;
    clGetProgramInfo (program, CL_PROGRAM_BINARY_SIZES, sizeof (size_t), &size, NULL);
    // trace("clGetProgramInfo (1) %d:\n", status );
    unsigned char  *binary = new unsigned char[size];
    clGetProgramInfo (program, CL_PROGRAM_BINARIES, size, &binary, NULL);
    // trace("clGetProgramInfo (2) %d:\n", status );
    FILE           *fpbin = fopen (name, "wb");

    if (fpbin == NULL) {
        fprintf (stderr, "Cannot create '%s'\n", name);

    } else {
        fwrite (binary, 1, size, fpbin);
        fclose (fpbin);
    }

    delete[]binary;
}

double
CLSolver::executionTime (cl_event gpuExecution) {
    cl_ulong        start,
                    end;
    clGetEventProfilingInfo (gpuExecution, CL_PROFILING_COMMAND_END, sizeof (cl_ulong),
                             &end, NULL);
    clGetEventProfilingInfo (gpuExecution, CL_PROFILING_COMMAND_START,
                             sizeof (cl_ulong), &start, NULL);
    const double    time = (double) 1.0e-9 * (end - start);
    return time;
}

cl_program
CLSolver::compileProgram (const char *const header_file,
                          const char *const kernel_file, cl_context cxContext,
                          cl_device_id d) {
    cl_program      cpProgram;
    size_t          program_length;
    char           *const source = readFile (kernel_file, &program_length);
    cl_int          ciErrNum;
    // Create the program for all GPUs in the context
    cpProgram =
        clCreateProgramWithSource (cxContext, 1, (const char **) &source,
                                   &program_length, &ciErrNum);
    free (source);
    UtilsCL::checkError (ciErrNum, "clCreateProgramWithSource");
    /*
     * Build program
     */
    char            clcompileflags[1024];
    sprintf (clcompileflags, "-cl-fast-relaxed-math -cl-mad-enable");
    ciErrNum = clBuildProgram (cpProgram, 0, NULL, clcompileflags, NULL, NULL);
    UtilsCL::checkError (ciErrNum, "clBuildProgram");

    if (ciErrNum != CL_SUCCESS) {
        char           *build_log;
        size_t          log_size;
        // First call to know the proper size
        ciErrNum =
            clGetProgramBuildInfo (cpProgram, d, CL_PROGRAM_BUILD_LOG, 0, NULL,
                                   &log_size);
        UtilsCL::checkError (ciErrNum, "clGetProgramBuildInfo 1");
        build_log = (char *) malloc ( (log_size + 1) );
        // Second call to get the log
        ciErrNum =
            clGetProgramBuildInfo (cpProgram, d, CL_PROGRAM_BUILD_LOG, log_size,
                                   build_log, NULL);
        UtilsCL::checkError (ciErrNum, "clGetProgramBuildInfo 2");
        build_log[log_size] = '\0';
        printf ("--- Build log extended kernel---\n ");
        fprintf (stderr, "%s\n", build_log);
        free (build_log);
    }

    return cpProgram;
}

char           *
CLSolver::readFile (const char *filename, size_t * length) {
    // locals
    FILE           *file = NULL;
    size_t          sourceLength;
    // open the OpenCL source code file
    file = fopen (filename, "rb");

    if (file == 0) {
        printf ("Can't open %s\n", filename);
        return NULL;
    }

    // get the length of the source code
    fseek (file, 0, SEEK_END);
    sourceLength = ftell (file);
    fseek (file, 0, SEEK_SET);
    // allocate a buffer for the source code string and read it in
    char           *sourceString = (char *) malloc (sourceLength + 1);
    int             ret = fread ( (sourceString), sourceLength, 1, file);

    if (ret == 0) {
        printf ("Can't read source %s\n", filename);
        return NULL;
    }

    // close the file and return the total length of the combined (preamble +
    // source) string
    fclose (file);

    if (length != 0) {
        *length = sourceLength;
    }

    sourceString[sourceLength] = '\0';
    return sourceString;
}

void
CLSolver::copyFromDevice (const cl_mem dMem, void *const hostPtr, const unsigned size,
                          cl_command_queue commandQueue, cl_event * gpuDone) {
    cl_int          ciErrNum =
        clEnqueueReadBuffer (commandQueue, dMem, CL_FALSE, 0, sizeof (double) * size,
                             hostPtr, 0, NULL, gpuDone);
    UtilsCL::checkError (ciErrNum, "clEnqueueReadBuffer");
}


void
CLSolver::runKernel (size_t localWorkSize, size_t globalWorkSize, cl_device_id device,
                     cl_command_queue commandQueue, cl_kernel kernel,
                     cl_event * gpuExecution) {
    cl_int          ciErrNum;
    ciErrNum =
        clEnqueueNDRangeKernel (commandQueue, kernel, 1, NULL, &globalWorkSize,
                                &localWorkSize, 0, NULL, gpuExecution);
    UtilsCL::checkError (ciErrNum, "clEnqueueNDRangeKernel");
}

#endif
