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

#include <localSolver.h>

#ifndef _TWO_OPT_LOCAL_SOLVER_
#define _TWO_OPT_LOCAL_SOLVER_

class           TwoOptLocalSolver: public LocalSolver {

public:

    TwoOptLocalSolver() {
    };

    TwoOptLocalSolver (city_coords * _coords) :
        LocalSolver (_coords) {
    };

    TwoOptLocalSolver (city_coords * _coords, cmdArguments * _args) :
        LocalSolver (_coords, _args) {
    };

    TwoOptLocalSolver (city_coords * _coords, cmdArguments * _args, string _description) :
        LocalSolver (_coords, _args,
                     _description) {
    };

    TwoOptLocalSolver (city_coords * _coords, cmdArguments * _args,
                       string _description, short _deviceId) : LocalSolver (_coords,
                                   _args,
                                   _description,
                                   _deviceId) {
    };

    TwoOptLocalSolver (city_coords * _coords, cmdArguments * _args,
                       string _description, short _deviceId,
                       ROUTE_DATA_TYPE _size) : LocalSolver (_coords, _args,
                                   _description, _deviceId,
                                   _size) {
    };

    TwoOptLocalSolver (city_coords * _coords, cmdArguments * _args,
                       string _description, short _deviceId, ROUTE_DATA_TYPE _size,
                       unsigned _tID) : LocalSolver (_coords, _args, _description,
                                   _deviceId, _size, _tID) {
    };

    ~TwoOptLocalSolver() {
        // printf("Thread %d ~TwoOptLocalSolver()\n", threadID);
    };

    void            optimize (vector < ROUTE_DATA_TYPE > &route,
                              vector < unsigned long >&bestLength);

};
#endif
