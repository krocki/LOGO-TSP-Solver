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
#include <localSolver.h>

#ifndef _ILS_GLOBAL_SOLVER_
#define _ILS_GLOBAL_SOLVER_

class           ILSGlobalSolver {


public:

    ILSGlobalSolver() {
        localSolver = NULL;
        description = "";
        coords = NULL;
        args = NULL;
        type = DEFAULT_GLOBAL_SOLVER;
        deviceId = -1;
        threadID = 0;
        solution = 1;
        timelimit = DEFAULT_TIMELIMIT;
        error = DEFAULT_ERROR_TOLERANCE;
        showLocalOptimizationInfo = DEFAULT_SHOW_LO_INFO;
    };

    ILSGlobalSolver (city_coords * _coords) {
        localSolver = NULL;
        coords = _coords;
        description = "";
        args = NULL;
        type = DEFAULT_GLOBAL_SOLVER;
        deviceId = -1;
        threadID = 0;
        solution = 1;
        timelimit = DEFAULT_TIMELIMIT;
        error = DEFAULT_ERROR_TOLERANCE;
        showLocalOptimizationInfo = DEFAULT_SHOW_LO_INFO;
    }

    ILSGlobalSolver (city_coords * _coords, cmdArguments * _args) {
        localSolver = NULL;
        coords = _coords;
        args = _args;
        description = "";
        type = DEFAULT_GLOBAL_SOLVER;
        deviceId = -1;
        threadID = 0;
        solution = 1;
        timelimit = DEFAULT_TIMELIMIT;
        error = DEFAULT_ERROR_TOLERANCE;
        showLocalOptimizationInfo = DEFAULT_SHOW_LO_INFO;
    }

    ILSGlobalSolver (city_coords * _coords, cmdArguments * _args, string _description) {
        localSolver = NULL;
        coords = _coords;
        args = _args;
        description = _description;
        type = DEFAULT_GLOBAL_SOLVER;
        deviceId = -1;
        threadID = 0;
        solution = 1;
        timelimit = DEFAULT_TIMELIMIT;
        error = DEFAULT_ERROR_TOLERANCE;
        showLocalOptimizationInfo = DEFAULT_SHOW_LO_INFO;
    }

    ILSGlobalSolver (city_coords * _coords, cmdArguments * _args, string _description,
                     short _type) {
        localSolver = NULL;
        coords = _coords;
        args = _args;
        description = _description;
        type = _type;
        deviceId = -1;
        threadID = 0;
        solution = 1;
        timelimit = DEFAULT_TIMELIMIT;
        error = DEFAULT_ERROR_TOLERANCE;
        showLocalOptimizationInfo = DEFAULT_SHOW_LO_INFO;
    }

    ILSGlobalSolver (city_coords * _coords, cmdArguments * _args, string _description,
                     short _type, ROUTE_DATA_TYPE _size) {
        localSolver = NULL;
        coords = _coords;
        args = _args;
        description = _description;
        type = _type;
        size = _size;
        deviceId = -1;
        threadID = 0;
        solution = 1;
        timelimit = DEFAULT_TIMELIMIT;
        error = DEFAULT_ERROR_TOLERANCE;
        showLocalOptimizationInfo = DEFAULT_SHOW_LO_INFO;
    }

    ILSGlobalSolver (city_coords * _coords, cmdArguments * _args, string _description,
                     short _type, ROUTE_DATA_TYPE _size, long _deviceId) {
        localSolver = NULL;
        coords = _coords;
        args = _args;
        description = _description;
        type = _type;
        size = _size;
        deviceId = _deviceId;
        threadID = 0;
        solution = 1;
        timelimit = DEFAULT_TIMELIMIT;
        error = DEFAULT_ERROR_TOLERANCE;
        showLocalOptimizationInfo = DEFAULT_SHOW_LO_INFO;
    }

    ILSGlobalSolver (city_coords * _coords, cmdArguments * _args, string _description,
                     short _type, ROUTE_DATA_TYPE _size, long _deviceId,
                     int _threadID) {
        localSolver = NULL;
        coords = _coords;
        args = _args;
        description = _description;
        type = _type;
        size = _size;
        deviceId = _deviceId;
        threadID = _threadID;
        solution = 1;
        timelimit = DEFAULT_TIMELIMIT;
        error = DEFAULT_ERROR_TOLERANCE;
        showLocalOptimizationInfo = DEFAULT_SHOW_LO_INFO;
    }

    // + args
    ILSGlobalSolver (city_coords * _coords, cmdArguments * _args, string _description,
                     short _type, ROUTE_DATA_TYPE _size, long _deviceId,
                     int _threadID, unsigned long _s, float _t, float _e, short _sL) {
        localSolver = NULL;
        coords = _coords;
        args = _args;
        description = _description;
        type = _type;
        size = _size;
        deviceId = _deviceId;
        threadID = _threadID;
        solution = _s;
        timelimit = _t;
        error = _e;
        showLocalOptimizationInfo = _sL;
    }



    virtual ~ ILSGlobalSolver() {
        // printf("~ILSGlobalSolver()\n");
    };

    virtual void    optimize (vector < ROUTE_DATA_TYPE > &route, int once = 0) = 0;

    process_time    benchmark (vector < ROUTE_DATA_TYPE > &route) {
        return localSolver->benchmark (route);
    };

    virtual void    init() = 0;
    virtual void    close() = 0;

    void            setTimeLimit (float t) {
        timelimit = t;
    };
    void            setError (float e) {
        error = e;
    };
    void            setShowLocalOptimizationInfo (short s) {
        showLocalOptimizationInfo = s;
    };

protected:

    cmdArguments * args;
    city_coords    *coords;
    string          description;
    short           type;
    ROUTE_DATA_TYPE size;
    LocalSolver    *localSolver;
    static          vector < vector < ROUTE_DATA_TYPE > >bestGlobalMinima;
    static          vector < unsigned long >bestGlobalMinimaLengths;

    long            deviceId;
    int             threadID;

    unsigned long   solution;
    float           timelimit;
    float           error;
    short           showLocalOptimizationInfo;

    void            randomPerturbation (vector < ROUTE_DATA_TYPE > &route);
    void            perturbation (vector < ROUTE_DATA_TYPE > &route, unsigned kicks);
    void            kick (vector < ROUTE_DATA_TYPE > &route);

    void            moveBack (vector < ROUTE_DATA_TYPE > &route);
    void            globalBackracking (vector < ROUTE_DATA_TYPE > &route);
};
#endif
