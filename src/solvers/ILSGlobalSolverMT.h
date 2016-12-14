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

#ifndef _ILS_GLOBAL_SOLVER_MT_
#define _ILS_GLOBAL_SOLVER_MT_
#ifdef HAVE_PTHREADS

class           ILSGlobalSolverMT: public ILSGlobalSolver {

public:

    ILSGlobalSolverMT (city_coords * _coords, cmdArguments * _args,
                       string _description, short _type, ROUTE_DATA_TYPE _size,
                       long _devId) : ILSGlobalSolver (_coords, _args, _description,
                                   _type, _size, _devId) {
        init();
        pthreads = DEFAULT_PTHREADS;
        setAffinity = DEFAULT_AFFINITY;
        maxCoresUsed = DEFAULT_MAX_CORES;
        comm = DEFAULT_PTHREADS_COMM;
    } ILSGlobalSolverMT (city_coords * _coords, cmdArguments * _args,
                         string _description, short _type, ROUTE_DATA_TYPE _size,
                         long _devId, int _id) : ILSGlobalSolver (_coords, _args,
                                     _description, _type,
                                     _size, _devId, _id) {
        init();
        pthreads = DEFAULT_PTHREADS;
        setAffinity = DEFAULT_AFFINITY;
        maxCoresUsed = DEFAULT_MAX_CORES;
        comm = DEFAULT_PTHREADS_COMM;
    }

    // + args
    ILSGlobalSolverMT (city_coords * _coords, cmdArguments * _args,
                       string _description, short _type, ROUTE_DATA_TYPE _size,
                       long _devId, int _id, unsigned long _s, float _t, float _e,
                       short _sL, unsigned _p, unsigned _sA, unsigned _m,
                       short _c) : ILSGlobalSolver (_coords, _args, _description, _type,
                                   _size, _devId, _id, _s, _t, _e,
                                   _sL) {
        init();
        pthreads = _p;
        setAffinity = _sA;
        maxCoresUsed = _m;
        comm = _c;
    }


    ~ILSGlobalSolverMT() {
        close();
        // printf("Thread %d ~ILSGlobalSolverSequential()\n", threadID);
    };

    void            init();
    void            close();
    void            optimize (vector < ROUTE_DATA_TYPE > &route, int once = 0);

    void            setComm (short _c) {
        comm = _c;
    };

protected:
    vector < unsigned long >bestLocalMinimaLengths;
    vector < vector < ROUTE_DATA_TYPE > >bestLocalMinima;

    unsigned        pthreads;
    unsigned        setAffinity;
    unsigned        maxCoresUsed;
    short           comm;
    // synchronization
    static pthread_mutex_t globalRouteMutex;

};
#endif
#endif
