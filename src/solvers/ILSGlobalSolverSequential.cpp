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

#include <ILSGlobalSolverSequential.h>
#include <cpuSolver.h>
#include <cpuMTSolver.h>
#ifdef HAVE_OPENCL
#include <CLSolver.h>
#endif
#ifdef HAVE_CUDA
#include <CUDASolver.h>
#endif

void
ILSGlobalSolverSequential::init() {
    std::stringstream sstm;
    sstm << description;

    if (type == TYPE_GLOBAL_SOLVER_CPU) {
        localSolver = new CPUSolver (coords, args, sstm.str(), deviceId, size);

    } else if (type == TYPE_GLOBAL_SOLVER_CPU_PARALLEL) {
        localSolver = new CPUMTSolver (coords, args, sstm.str(), deviceId, size);

    } else if (type == TYPE_GLOBAL_SOLVER_CL) {
#ifdef HAVE_OPENCL
        localSolver = new CLSolver (coords, args, sstm.str(), deviceId, size);
#else
        trace ("OpenCL code doesn't exist!\n");
        exit (-1);
#endif

    } else if (type == TYPE_GLOBAL_SOLVER_CUDA) {
#ifdef HAVE_CUDA
        localSolver = new CUDASolver (coords, args, sstm.str(), deviceId, size);
#else
        trace ("CUDA code doesn't exist!\n");
        exit (-1);
#endif

    } else {
        trace ("Unknown ILS Global Solver type!\n");
    }
}

void
ILSGlobalSolverSequential::optimize (vector < ROUTE_DATA_TYPE > &route, int once) {
    unsigned long   temp;
    INIT_TRACEF temp = routeLength (route, coords);
    bestGlobalMinimaLengths.push_back (temp);
    bestGlobalMinima.push_back (route);
    trace ("[%s] Initial minimum (%ld) -> Length: %ld = %.5f%% of the target\n",
           localSolver->getDescription().c_str(), bestGlobalMinima.size(),
           bestGlobalMinimaLengths.back(),
           100.0 * (double) bestGlobalMinimaLengths.back() / (double) solution);
    tracef ("%ld, %.5f\n", bestGlobalMinimaLengths.back(),
            100.0 * (double) bestGlobalMinimaLengths.back() / (double) solution);

    do {
        localSolver->optimize (route, bestGlobalMinimaLengths);
        temp = routeLength (route, coords);

        if (temp < bestGlobalMinimaLengths.back() ) {
            bestGlobalMinimaLengths.push_back (temp);
            bestGlobalMinima.push_back (route);
            trace
            ("[%s] New global minimum (%ld) -> Length: %ld = %.5f%% of the target\n",
             localSolver->getDescription().c_str(), bestGlobalMinima.size(),
             bestGlobalMinimaLengths.back(),
             100.0 * (double) bestGlobalMinimaLengths.back() /
             (double) solution);
            tracef ("%ld, %.5f\n", bestGlobalMinimaLengths.back(),
                    100.0 * (double) bestGlobalMinimaLengths.back() /
                    (double) solution);

        } else {
            route = bestGlobalMinima.back();
        }

        if (once == 1)
            break;

        randomPerturbation (route);
    } while ( (solution * (1.0f + error) < bestGlobalMinimaLengths.back() )
              && (timelimit == 0 || (getTotalTime() < timelimit) ) );
}

void
ILSGlobalSolverSequential::close() {
    delete (localSolver);
}
