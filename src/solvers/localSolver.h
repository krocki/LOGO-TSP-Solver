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
#include <defs.h>

#ifndef _LOCAL_SOLVER_
#define _LOCAL_SOLVER_


class           LocalSolver {

public:

    LocalSolver() {
        deviceId = 0;
        description = "";
        coords = NULL;
        args = NULL;
        size = 0;
        threadID = 0;
    };

    LocalSolver (city_coords * _coords) {
        coords = _coords;
        deviceId = 0;
        args = NULL;
        description = "";
        size = 0;
        threadID = 0;
    };

    LocalSolver (city_coords * _coords, cmdArguments * _args) {
        coords = _coords;
        args = _args;
        deviceId = 0;
        description = "";
        size = 0;
        threadID = 0;
    };

    LocalSolver (city_coords * _coords, cmdArguments * _args, string _description) {
        coords = _coords;
        args = _args;
        description = _description;
        deviceId = 0;
        size = 0;
        threadID = 0;
    };

    LocalSolver (city_coords * _coords, cmdArguments * _args, string _description,
                 long _deviceId) {
        coords = _coords;
        args = _args;
        description = _description;
        deviceId = _deviceId;
        size = 0;
        threadID = 0;
    };

    LocalSolver (city_coords * _coords, cmdArguments * _args, string _description,
                 long _deviceId, ROUTE_DATA_TYPE _size) {
        coords = _coords;
        args = _args;
        description = _description;
        deviceId = _deviceId;
        size = _size;
        threadID = 0;
    };

    LocalSolver (city_coords * _coords, cmdArguments * _args, string _description,
                 long _deviceId, ROUTE_DATA_TYPE _size, unsigned _tID) {
        coords = _coords;
        args = _args;
        description = _description;
        deviceId = _deviceId;
        size = _size;
        threadID = _tID;
    };


    void            setArgs (cmdArguments * _args) {
        args = _args;
    };

    void            setDescription (char *_description) {
        description = _description;
    };

    std::string getDescription (void) {
        return description;
    };

    void            setDeviceId (long _deviceId) {
        deviceId = _deviceId;
    };

    virtual ~ LocalSolver() {
        // printf("Thread %d ~LocalSolver()\n", threadID);
    };

    virtual void    optimize (vector < ROUTE_DATA_TYPE > &route,
                              vector < unsigned long >&bestLength) = 0;
    virtual void    init() = 0;
    virtual void    close() = 0;
    virtual struct process_time benchmark (vector < ROUTE_DATA_TYPE > &route) = 0;

protected:

    virtual struct best2_out optimizeStep (const vector < ROUTE_DATA_TYPE > &route) =
        0;
    cmdArguments   *args;
    city_coords    *coords;
    std::string description;
    long            deviceId;
    ROUTE_DATA_TYPE size;
    unsigned        threadID;
};
#endif
