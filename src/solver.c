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
#include <ILSGlobalSolverSequential.h>
#include <ILSGlobalSolverMT.h>
#ifdef HAVE_OPENCL
#include <CLSolver.h>
#include <UtilsCL.h>
#endif
#ifdef HAVE_CUDA
#include <CUDASolver.h>
#include <UtilsCUDA.h>
#endif

vector < deviceInfo > selectDevices (cmdArguments * args, unsigned short method);

void
run (cmdArguments * args) {
    city_coords    *coords;
    vector < ROUTE_DATA_TYPE > route;
    ROUTE_DATA_TYPE cities = 0;
    coords = (city_coords *) ALLOC (sizeof (city_coords) * MAX_CITIES);

    if (coords == NULL) {
        fprintf (stderr, "go:coords:Could not allocate that much memory (%ld B)",
                 sizeof (city_coords) * MAX_CITIES);
        exit (-1);

    } else {
        trace ("Allocated %.2f kB for coordinates\n",
               (double) (sizeof (city_coords) * MAX_CITIES) / 1024.0);
    }

    trace ("Reading %s...\n", args->filename);
    cities = readFileCoords (args->filename, coords);
    trace ("Done (%ld coordinates loaded).\n", (long) cities);
    // reset the timer
    initStartTime();

    if ( (!strcmp (args->initMethod, "mf") ) || (!strcmp (args->initMethod, "nn") ) ) {
        trace ("Constructing kd-tree...\n");
        // build kd-tree
        constructKDTree (coords, cities);
        trace ("Done.\n");
    }

    if (args->initRouteFromFile == 1) {
        routeInitFromFile (route, cities, args->initRouteFile);
    }

    if (!strcmp (args->initMethod, "mf") ) {
        trace ("Using multiple fragment construction heuristic...\n");
        routeInit (route, ROUTE_INIT_MF, cities, coords);

    } else if (!strcmp (args->initMethod, "nn") ) {
        trace ("Using nearest neighbor construction heuristic...\n");
        routeInit (route, ROUTE_INIT_NN, cities, coords);

    } else if (!strcmp (args->initMethod, "random") ) {
        trace ("Using a random initial tour...\n");
        routeInit (route, ROUTE_INIT_SHUFFLE, cities, coords);

    } else {
        printf ("Unrecognized route init type: %s\n", args->initMethod);
        free (coords);
        exit (-1);
    }

    // printRoute(route, coords);
    // check if the tour is valid
    checkTour (route);
    initStartTime();

    if (args->mode == MODE_CPU ||
            args->mode == MODE_OPENCL ||
            args->mode == MODE_CUDA ||
            args->mode == MODE_ALL ||
            args->mode == MODE_CPU_ALL ||
            args->mode == MODE_CPU_PARALLEL || args->mode == MODE_CPU_PARALLEL_HYBRID) {
        if (args->benchmark) {
            benchmark (args, coords, route, args->mode, args->benchmark);

        } else {
            if (args->mode == MODE_CPU_PARALLEL_HYBRID) {
                args->mode = MODE_CPU_PARALLEL;
                route = optimize (args, coords, route, args->mode, 1);
                args->mode = MODE_CPU_ALL;
            }

#ifdef HAVE_PTHREADS
            optimizeThreaded (args, coords, route, args->mode);
#else
            optimize (args, coords, route, args->mode);
#endif
        }

    } else {
        printf ("Unrecognized mode: %d\n", args->mode);
        free (coords);
        exit (-1);
    }

    // free CPU memory
    free (coords);
}

vector < ROUTE_DATA_TYPE > optimize (cmdArguments * args, city_coords * coords,
                                     vector < ROUTE_DATA_TYPE > route,
                                     unsigned short method, int once) {
    ILSGlobalSolver *solver = NULL;
    vector < deviceInfo > selectedDevices = selectDevices (args, method);

    if (method == MODE_CPU) {
        solver =
            new ILSGlobalSolverSequential (coords, args, getCPUInfo(),
                                           TYPE_GLOBAL_SOLVER_CPU, route.size(),
                                           selectedDevices[0].localNum,
                                           args->solution, args->timelimit,
                                           args->error,
                                           args->showLocalOptimizationInfo);

    } else if (method == MODE_CPU_PARALLEL) {
        std::stringstream sstm;
        sstm << args->maxCoresUsed << " x " << getCPUInfo();
        solver =
            new ILSGlobalSolverSequential (coords, args, sstm.str(),
                                           TYPE_GLOBAL_SOLVER_CPU_PARALLEL,
                                           route.size(), selectedDevices[0].localNum,
                                           args->solution, args->timelimit,
                                           args->error,
                                           args->showLocalOptimizationInfo);

    } else if (method == MODE_CUDA) {
        solver =
            new ILSGlobalSolverSequential (coords, args, "CUDA| ",
                                           TYPE_GLOBAL_SOLVER_CUDA, route.size(),
                                           selectedDevices[0].localNum,
                                           args->solution, args->timelimit,
                                           args->error,
                                           args->showLocalOptimizationInfo);

    } else if (method == MODE_OPENCL) {
        solver =
            new ILSGlobalSolverSequential (coords, args, "OPENCL| ",
                                           TYPE_GLOBAL_SOLVER_CL, route.size(),
                                           selectedDevices[0].localNum,
                                           args->solution, args->timelimit,
                                           args->error,
                                           args->showLocalOptimizationInfo);
    }

    if (solver != NULL) {
        // initStartTime();
        solver->optimize (route, once);
        delete (solver);
    }

    return route;
}

#ifdef HAVE_PTHREADS

vector < ROUTE_DATA_TYPE > optimizeThreaded (cmdArguments * args,
        city_coords * coords,
        vector < ROUTE_DATA_TYPE > route,
        unsigned short method) {
    int             ret;
    threadData      messages[MAX_THREADS];
    pthread_t       threads[MAX_THREADS];
    vector < deviceInfo > selectedDevices = selectDevices (args, method);
    initStartTime();
    trace ("Launching threads...\n");

    for (unsigned int i = 0; i < selectedDevices.size(); i++) {
        messages[i].id = i;
        messages[i].dev = selectedDevices[i].localNum;
        messages[i].route = route;
        messages[i].args = args;
        messages[i].coords = coords;

        if (selectedDevices[i].typeId == TYPE_GLOBAL_SOLVER_CPU) {
            messages[i].method = MODE_CPU;

        } else if (selectedDevices[i].typeId == TYPE_GLOBAL_SOLVER_CPU_PARALLEL) {
            messages[i].method = MODE_CPU_PARALLEL;

        } else if (selectedDevices[i].typeId == TYPE_GLOBAL_SOLVER_CUDA) {
            messages[i].method = MODE_CUDA;

        } else if (selectedDevices[i].typeId == TYPE_GLOBAL_SOLVER_CL) {
            messages[i].method = MODE_OPENCL;
        }

        ret =
            pthread_create (&threads[i], NULL, optimizationThread,
                            (void *) &messages[i]);

        if (ret) {
            printf ("ERROR; return code from pthread_create() is %d\n", ret);
            exit (-1);
        }
    }

    // end threads
    for (unsigned int i = 0; i < args->pthreads; i++) {
        pthread_join (threads[i], NULL);
    }

    trace ("All threads have finished.\n");
    return route;
}

void           *
optimizationThread (void *data) {
    threadData      message = * ( (threadData *) (data) );
    unsigned        id = message.id;
    unsigned        dev = message.dev;
    city_coords    *coords = message.coords;
    cmdArguments   *args = message.args;
    vector < ROUTE_DATA_TYPE > route = message.route;
    int             method = message.method;
    std::stringstream sstm;
    ILSGlobalSolver *solver = NULL;

    if (method == MODE_CPU) {
        sstm << getCPUInfo();
        solver =
            new ILSGlobalSolverMT (coords, args, getCPUInfo(), TYPE_GLOBAL_SOLVER_CPU,
                                   route.size(), dev, id, args->solution,
                                   args->timelimit, args->error,
                                   args->showLocalOptimizationInfo, args->pthreads,
                                   args->setAffinity, args->maxCoresUsed, args->comm);

    } else if (method == MODE_CPU_PARALLEL) {
        std::stringstream sstm;
        sstm << args->maxCoresUsed << " x " << getCPUInfo();
        solver =
            new ILSGlobalSolverMT (coords, args, sstm.str(),
                                   TYPE_GLOBAL_SOLVER_CPU_PARALLEL, route.size(), dev,
                                   id, args->solution, args->timelimit, args->error,
                                   args->showLocalOptimizationInfo, args->pthreads,
                                   args->setAffinity, args->maxCoresUsed, args->comm);

    } else if (method == MODE_CUDA) {
        sstm << "CUDA| ";
        solver =
            new ILSGlobalSolverMT (coords, args, sstm.str(), TYPE_GLOBAL_SOLVER_CUDA,
                                   route.size(), dev, id, args->solution,
                                   args->timelimit, args->error,
                                   args->showLocalOptimizationInfo, args->pthreads,
                                   args->setAffinity, args->maxCoresUsed, args->comm);

    } else if (method == MODE_OPENCL) {
        sstm << "OpenCL| ";
        solver =
            new ILSGlobalSolverMT (coords, args, sstm.str(), TYPE_GLOBAL_SOLVER_CL,
                                   route.size(), dev, id, args->solution,
                                   args->timelimit, args->error,
                                   args->showLocalOptimizationInfo, args->pthreads,
                                   args->setAffinity, args->maxCoresUsed, args->comm);
    }

    if (solver != NULL) {
        solver->optimize (route);
        delete (solver);
    }

    pthread_exit (NULL);
}

#endif

void
benchmark (cmdArguments * args, city_coords * coords,
           vector < ROUTE_DATA_TYPE > route, unsigned short method,
           int BENCHMARK_ITERS) {
    ILSGlobalSolver *solver = NULL;
    vector < deviceInfo > selectedDevices = selectDevices (args, method);
    ROUTE_DATA_TYPE cities = route.size();
    unsigned long long totalFlops;
    unsigned long long total2opts = (route.size() - 2) * (route.size() - 3);

    // estimate the FLOPs...
    if (method == TYPE_GLOBAL_SOLVER_CPU || method == TYPE_GLOBAL_SOLVER_CPU_PARALLEL
            || method == TYPE_GLOBAL_SOLVER_CPU_PARALLEL_HYBRID) {
        totalFlops =
            (3 * CPU_FLOPS_PER_DISTANCE * total2opts) / 2 +
            CPU_FLOPS_PER_DISTANCE * (route.size() - 3);

    } else if (cities <= MAX_CITIES) {
        totalFlops = (36 * total2opts) / 2;

    } else {
        totalFlops = (31 * total2opts) / 2;
    }

    if (method == MODE_CPU) {
        solver =
            new ILSGlobalSolverSequential (coords, args, getCPUInfo(),
                                           TYPE_GLOBAL_SOLVER_CPU, route.size(),
                                           selectedDevices[0].localNum,
                                           args->solution, args->timelimit,
                                           args->error,
                                           args->showLocalOptimizationInfo);

    } else if (method == MODE_CPU_PARALLEL) {
        std::stringstream sstm;
        sstm << args->maxCoresUsed << " x " << getCPUInfo();
        solver =
            new ILSGlobalSolverSequential (coords, args, sstm.str(),
                                           TYPE_GLOBAL_SOLVER_CPU_PARALLEL,
                                           route.size(), selectedDevices[0].localNum,
                                           args->solution, args->timelimit,
                                           args->error,
                                           args->showLocalOptimizationInfo);

    } else if (method == MODE_CUDA) {
        solver =
            new ILSGlobalSolverSequential (coords, args, "CUDA| ",
                                           TYPE_GLOBAL_SOLVER_CUDA, route.size(),
                                           selectedDevices[0].localNum,
                                           args->solution, args->timelimit,
                                           args->error,
                                           args->showLocalOptimizationInfo);

    } else if (method == MODE_OPENCL) {
        solver =
            new ILSGlobalSolverSequential (coords, args, "OPENCL| ",
                                           TYPE_GLOBAL_SOLVER_CL, route.size(),
                                           selectedDevices[0].localNum,
                                           args->solution, args->timelimit,
                                           args->error,
                                           args->showLocalOptimizationInfo);
    }

    if (solver != NULL) {
        for (int i = 0; i < BENCHMARK_ITERS; i++) {
            process_time    T = solver->benchmark (route);
            trace
            ("Total Time = %.6f s (HtD = %.6f s, kernel = %.6f s, DtH = %.6f s)\n",
             T.HtD_time + T.kernel_time + T.DtH_time, T.HtD_time, T.kernel_time,
             T.DtH_time);
            trace ("GFLOPS = %.2f\n",
                   (double) totalFlops / (double) (1000.0 * 1000.0 * 1000.0 *
                                                   (T.kernel_time) ) );
            trace ("2-opt checks/s = %.2f M\n",
                   (double) total2opts / (double) (1000.0 * 1000.0 *
                                                   (T.kernel_time) ) );
        }

        delete (solver);
    }
}

vector < deviceInfo > selectDevices (cmdArguments * args, unsigned short method) {
    vector < deviceInfo > availableDevices;
    vector < deviceInfo > selectedDevices;
    deviceInfo      temp;

    // query cpu

    if (method == MODE_CPU || method == MODE_CPU_PARALLEL || method == MODE_CPU_ALL
            || method == MODE_ALL || method == MODE_CPU_PARALLEL_HYBRID) {
        trace ("Querying CPU...\n");
        temp.name = getCPUInfo();
        temp.localNum = 0;

        if (method == MODE_CPU || method == MODE_CPU_ALL || method == MODE_ALL) {
            temp.type = string ("CPU");
            temp.typeId = TYPE_GLOBAL_SOLVER_CPU;
            availableDevices.push_back (temp);
        }

        if (method == MODE_CPU_PARALLEL || method == MODE_ALL
                || method == MODE_CPU_PARALLEL_HYBRID) {
            temp.localNum = 0;
            std::stringstream sstm;
            sstm << args->maxCoresUsed << " x " << temp.name;
            temp.name = sstm.str();
            temp.type = string ("CPU PARALLEL");
            temp.typeId = TYPE_GLOBAL_SOLVER_CPU_PARALLEL;
            availableDevices.push_back (temp);
        }
    }

    // query opencl
#ifdef HAVE_OPENCL

    if (method == MODE_OPENCL || method == MODE_ALL) {
        trace ("Querying OpenCL...\n");
        vector < deviceInfo > clDevices = UtilsCL::listDevices();
        availableDevices.insert (availableDevices.end(), clDevices.begin(),
                                 clDevices.end() );
    }

#endif
    // query cuda
#ifdef HAVE_CUDA

    if (method == MODE_CUDA || method == MODE_ALL) {
        trace ("Querying CUDA...\n");
        vector < deviceInfo > cudaDevices = UtilsCUDA::listDevices();
        availableDevices.insert (availableDevices.end(), cudaDevices.begin(),
                                 cudaDevices.end() );
    }

#endif

    for (unsigned int i = 0; i < availableDevices.size(); i++) {
        trace ("[%2d]: %s [%s, local id = %ld]\n", i,
               availableDevices[i].name.c_str(), availableDevices[i].type.c_str(),
               availableDevices[i].localNum);
    }

    if (args->autoDevice == 1) {
        for (unsigned t = 0; t < availableDevices.size(); t++) {
            trace ("Auto Selected [%2d]: %s [%s, local id = %ld]\n", t,
                   availableDevices[t].name.c_str(), availableDevices[t].type.c_str(),
                   availableDevices[t].localNum);
            selectedDevices.push_back (availableDevices[t]);

            if (selectedDevices.size() >= args->maxDevices && args->maxDevices != -1)
                break;
        }

    } else if (args->device < 0 || args->device >= availableDevices.size() ) {
        trace ("Select device(s) (space separated): ");
        std::string input;
        std::getline (std::cin, input);
        istringstream   iss (input);
        int             count = 0;
        vector < int   >tokens;

        // process tokens
        while (iss) {
            int             sub;
            iss >> sub;

            if (iss && sub >= 0 && sub < availableDevices.size() ) {
                tokens.push_back (sub);
            }

            count = count + 1;
        }

        if (tokens.empty() ) {
            tokens.push_back (0);
        }

        // sort
        sort (tokens.begin(), tokens.end() );

        // and remove duplicates
        // allow/forbid multiple threads using the same device
        // tokens.erase( unique( tokens.begin(), tokens.end() ), tokens.end() );

        for (unsigned t = 0; t < tokens.size(); t++) {
            trace ("Selected [%2d]: %s [%s, local id = %ld]\n", tokens[t],
                   availableDevices[tokens[t]].name.c_str(),
                   availableDevices[tokens[t]].type.c_str(),
                   availableDevices[tokens[t]].localNum);
            selectedDevices.push_back (availableDevices[tokens[t]]);
        }

    } else {
        if (method == MODE_CPU_ALL) {
            trace ("Using %d CPU cores\n", args->maxCoresUsed);

            for (int i = 0; i < args->maxCoresUsed; i++)
                selectedDevices.push_back (availableDevices[args->device]);
        }

        trace ("Selected [%2d]: %s [%s, local id = %ld]\n", args->device,
               availableDevices[args->device].name.c_str(),
               availableDevices[args->device].type.c_str(),
               availableDevices[args->device].localNum);

        if (method != MODE_CPU_ALL)
            selectedDevices.push_back (availableDevices[args->device]);
    }

    return selectedDevices;
}
