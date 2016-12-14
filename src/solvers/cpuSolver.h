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
#include <cpusimd.h>

#ifndef _CPU_SOLVER_
#define _CPU_SOLVER_

class           CPUSolver: public TwoOptLocalSolver {

public:

    CPUSolver() {
        init();
    };

    CPUSolver (city_coords * _coords) :
        TwoOptLocalSolver (_coords) {
        init();
    };

    CPUSolver (city_coords * _coords, cmdArguments * _args) :
        TwoOptLocalSolver (_coords, _args) {
        init();
    };

    CPUSolver (city_coords * _coords, cmdArguments * _args, string _description) :
        TwoOptLocalSolver (_coords, _args, _description) {
        init();
    };

    CPUSolver (city_coords * _coords, cmdArguments * _args, string _description,
               short _deviceId) : TwoOptLocalSolver (_coords, _args, _description,
                           _deviceId) {
        init();
    };

    CPUSolver (city_coords * _coords, cmdArguments * _args, string _description,
               short _deviceId, ROUTE_DATA_TYPE _size) : TwoOptLocalSolver (_coords,
                           _args,
                           _description,
                           _deviceId,
                           _size) {
        init();
    };

    CPUSolver (city_coords * _coords, cmdArguments * _args, string _description,
               short _deviceId, ROUTE_DATA_TYPE _size,
               unsigned _tID) : TwoOptLocalSolver (_coords, _args, _description,
                           _deviceId, _size, _tID) {
        init();
    };

    virtual ~ CPUSolver() {
        // printf("Thread %d ~CPUSolver()\n", threadID);
        close();
    };

    void            init();
    void            close();
    // virtual void optimize(vector<ROUTE_DATA_TYPE> &route, unsigned long
    // bestLength);
    virtual struct process_time benchmark (vector < ROUTE_DATA_TYPE > &route);

protected:
    virtual struct best2_out optimizeStep (const vector < ROUTE_DATA_TYPE > &route);

};
#endif
