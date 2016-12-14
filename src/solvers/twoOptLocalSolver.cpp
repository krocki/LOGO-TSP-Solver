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

// Implementation of the 2-opt solver

void
TwoOptLocalSolver::optimize (vector < ROUTE_DATA_TYPE > &route,
                             vector < unsigned long >&bestLength) {

    struct best2_out out;
    register int    best_i,
             best_j,
             best_change;
    unsigned long   newLength;
    INIT_TRACEF

    do {

        out = optimizeStep (route);
        best_change = out.minchange;
        best_i = out.i;
        best_j = out.j;

        // perform the swap
        if (best_j > best_i) {
            swapint (best_j, best_i);
        }

        if (best_change < 0) {
            rotate (route.begin(), route.begin() + (best_j), route.end() );
            reverse (route.begin() + (best_i - best_j), route.end() );
        }

        newLength = routeLength (route, coords);

        if (args->showLocalOptimizationInfo)
            if (bestLength.size() > 0)
                if (newLength < bestLength.back() && best_change < 0) {
                    trace
                    ("[%s] Local Optimization: 2-opt pair found (%d,%d) -> change %d, New length: %ld, %.5f%% of the known optimum\n",
                     description.c_str(), best_i, best_j, best_change, newLength,
                     100.0 * (double) newLength / (double) args->solution);
                    tracef ("%ld, %.5f\n", newLength,
                            100.0 * (double) newLength / (double) args->solution);
                }

    } while (best_change < 0);
};
