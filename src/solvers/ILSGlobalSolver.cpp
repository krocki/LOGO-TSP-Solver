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

#include <ILSGlobalSolver.h>

vector < vector < ROUTE_DATA_TYPE > >ILSGlobalSolver::bestGlobalMinima;
vector < unsigned long >
ILSGlobalSolver::bestGlobalMinimaLengths;

void
ILSGlobalSolver::randomPerturbation (vector < ROUTE_DATA_TYPE > &route) {
    // perturbation(route, rand() % ((unsigned int)sqrt(route.size()) - 2) + 2);
    perturbation (route, 1);
}

void
ILSGlobalSolver::perturbation (vector < ROUTE_DATA_TYPE > &route, unsigned kicks) {
    for (unsigned k = 0; k < kicks; k++) {
        kick (route);
    }
}

void
ILSGlobalSolver::kick (vector < ROUTE_DATA_TYPE > &route) {
    ROUTE_DATA_TYPE a = rand() % route.size();
    ROUTE_DATA_TYPE b = rand() % route.size();

    if (b < a) {
        swapint (a, b);
    }

    rotate (route.begin(), route.begin() + a, route.end() );
    reverse (route.begin() + (b - a), route.end() );
}
