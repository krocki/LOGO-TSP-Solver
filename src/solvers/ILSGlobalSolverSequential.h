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

#ifndef _ILS_GLOBAL_SOLVER_SEQUENTIAL_
#define _ILS_GLOBAL_SOLVER_SEQUENTIAL_

class           ILSGlobalSolverSequential: public ILSGlobalSolver {

public:

    ILSGlobalSolverSequential (city_coords * _coords, cmdArguments * _args,
                               string _description, short _type,
                               ROUTE_DATA_TYPE _size) : ILSGlobalSolver (_coords, _args,
                                           _description,
                                           _type, _size) {
        init();
    } ILSGlobalSolverSequential (city_coords * _coords, cmdArguments * _args,
                                 string _description, short _type,
                                 ROUTE_DATA_TYPE _size,
                                 long _id) : ILSGlobalSolver (_coords, _args,
                                             _description, _type, _size,
                                             _id) {
        init();
    }

    // + args
    ILSGlobalSolverSequential (city_coords * _coords, cmdArguments * _args,
                               string _description, short _type,
                               ROUTE_DATA_TYPE _size, long _devId, unsigned long _s,
                               float _t, float _e, short _sL) : ILSGlobalSolver (_coords,
                                           _args,
                                           _description,
                                           _type,
                                           _size,
                                           _devId,
                                           0, _s,
                                           _t, _e,
                                           _sL) {
        init();
    }



    ~ILSGlobalSolverSequential() {
        close();
        // printf("~ILSGlobalSolverSequential()\n");
    };

    void            init();
    void            close();
    void            optimize (vector < ROUTE_DATA_TYPE > &route, int once = 0);
};
#endif
