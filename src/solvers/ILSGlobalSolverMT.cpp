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

#include <ILSGlobalSolverMT.h>
#include <cpuSolver.h>
#include <cpuMTSolver.h>
#ifdef HAVE_OPENCL
#include <CLSolver.h>
#endif
#ifdef HAVE_CUDA
#include <CUDASolver.h>
#endif

#ifdef HAVE_PTHREADS

pthread_mutex_t
ILSGlobalSolverMT::globalRouteMutex = PTHREAD_MUTEX_INITIALIZER;

void
ILSGlobalSolverMT::init() {
    std::stringstream sstm;
    sstm << "#" << std::setfill ('0') << std::
         setw (2) << threadID << "| " << description;

    if (type == TYPE_GLOBAL_SOLVER_CPU) {
        localSolver =
            new CPUSolver (coords, args, sstm.str(), deviceId, size, threadID);

    } else if (type == TYPE_GLOBAL_SOLVER_CPU_PARALLEL) {
        localSolver =
            new CPUMTSolver (coords, args, sstm.str(), deviceId, size, threadID);

    } else if (type == TYPE_GLOBAL_SOLVER_CL) {
#ifdef HAVE_OPENCL
        localSolver =
            new CLSolver (coords, args, sstm.str(), deviceId, size, threadID);
#else
        trace ("OpenCL code doesn't exist!\n");
        exit (-1);
#endif

    } else if (type == TYPE_GLOBAL_SOLVER_CUDA) {
#ifdef HAVE_CUDA
        localSolver =
            new CUDASolver (coords, args, sstm.str(), deviceId, size, threadID);
#else
        trace ("CUDA code doesn't exist!\n");
        exit (-1);
#endif

    } else {
        trace ("Unknown ILS Global Solver type!\n");
    }

#ifdef __linux__

    if (setAffinity == 1) {
        cpu_set_t       mask;
        CPU_ZERO (&mask);
        CPU_SET (threadID % maxCoresUsed, &mask);

        if (sched_setaffinity (0, sizeof (mask), &mask) < 0) {
            perror ("sched_setaffinity");
        }

        trace ("Thread %d, assigned to core %d\n", threadID, threadID % maxCoresUsed);
    }

#endif
}

void
ILSGlobalSolverMT::optimize (vector < ROUTE_DATA_TYPE > &route, int once) {
    unsigned long   temp;
    INIT_TRACEF temp = routeLength (route, coords);
    bestLocalMinimaLengths.push_back (temp);
    bestLocalMinima.push_back (route);

    // initial solution
    if (bestGlobalMinimaLengths.empty() ) {
        pthread_mutex_lock (&globalRouteMutex);

        if (bestGlobalMinimaLengths.empty() ) {
            bestGlobalMinimaLengths.push_back (temp);
            bestGlobalMinima.push_back (route);

            if (comm == 1) {
                trace
                ("[%s] Initial global minimum (%ld) -> Length: %ld = %.5f%% of the target\n",
                 localSolver->getDescription().c_str(), bestGlobalMinima.size(),
                 bestGlobalMinimaLengths.back(),
                 100.0 * (double) bestGlobalMinimaLengths.back() /
                 (double) solution);
                tracef ("%ld, %.5f\n", bestGlobalMinimaLengths.back(),
                        100.0 * (double) bestGlobalMinimaLengths.back() /
                        (double) solution);
            }
        }

        pthread_mutex_unlock (&globalRouteMutex);
    }

    if (comm != 1) {
        trace
        ("[%s] Initial local minimum (%ld) -> Length: %ld = %.5f%% of the target\n",
         localSolver->getDescription().c_str(), bestLocalMinima.size(),
         bestLocalMinimaLengths.back(),
         100.0 * (double) bestLocalMinimaLengths.back() / (double) solution);
        tracef ("%ld, %.5f\n", bestLocalMinimaLengths.back(),
                100.0 * (double) bestLocalMinimaLengths.back() / (double) solution);
    }

    do {
        if (comm == 1) {
            // read the best solution
            if (bestGlobalMinimaLengths.back() < bestGlobalMinimaLengths.back() ) {
                bestGlobalMinimaLengths.push_back (bestGlobalMinimaLengths.back() );
                bestGlobalMinima.push_back (bestGlobalMinima.back() );
                route = bestGlobalMinima.back();
            }
        }

        localSolver->optimize (route, bestGlobalMinimaLengths);
        temp = routeLength (route, coords);

        if (temp < bestLocalMinimaLengths.back() ) {
            bestLocalMinimaLengths.push_back (temp);
            bestLocalMinima.push_back (route);

            if (comm != 1 && !args->showLocalOptimizationInfo) {
                trace
                ("[%s] New local minimum (%ld) -> Length: %ld = %.5f%% of the target\n",
                 localSolver->getDescription().c_str(), bestLocalMinima.size(),
                 bestLocalMinimaLengths.back(),
                 100.0 * (double) bestLocalMinimaLengths.back() /
                 (double) solution);
                tracef ("%ld, %.5f\n", bestLocalMinimaLengths.back(),
                        100.0 * (double) bestLocalMinimaLengths.back() /
                        (double) solution);
            }

            if (temp < bestGlobalMinimaLengths.back() ) {
                pthread_mutex_lock (&globalRouteMutex);

                if (temp < bestGlobalMinimaLengths.back() ) {
                    bestGlobalMinimaLengths.push_back (temp);
                    bestGlobalMinima.push_back (route);

                    if ( (comm == 1 || args->pthreads > 1)
                            && args->showLocalOptimizationInfo) {
                        trace
                        ("[%s] New global minimum (%ld) -> Length: %ld = %.5f%% of the target\n",
                         localSolver->getDescription().c_str(),
                         bestGlobalMinima.size(), bestGlobalMinimaLengths.back(),
                         100.0 * (double) bestGlobalMinimaLengths.back() /
                         (double) solution);
                        tracef ("%ld, %.5f\n", bestGlobalMinimaLengths.back(),
                                100.0 * (double) bestGlobalMinimaLengths.back() /
                                (double) solution);
                    }
                }

                pthread_mutex_unlock (&globalRouteMutex);
            }

        } else {
            if (comm == 1) {
                route = bestGlobalMinima.back();

            } else {
                route = bestLocalMinima.back();
            }
        }

        if (once == 1)
            break;

        randomPerturbation (route);
    } while ( (solution * (1.0f + error) < bestGlobalMinimaLengths.back() )
              && (timelimit == 0 || (getTotalTime() < timelimit) ) );
}

void
ILSGlobalSolverMT::close() {
    delete (localSolver);
}
#endif
