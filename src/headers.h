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

#include <stdio.h>
#include <stdlib.h>
#ifdef __linux__
#include <malloc.h>
#endif
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <math.h>
#include <vector>
#include <iostream>		// std::cout, std::endl
#include <iomanip>		// std::setw
#include <fstream>
#include <set>
#include <ext/numeric>
#include <algorithm>
#include <queue>
#include <limits.h>
#include <defs.h>
#include <sstream>
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif
using namespace std;

#ifndef _HEADERS_H_
#define _HEADERS_H_
// cut.c
extern cmdArguments parseInput (int argc, const char **argv);

// common.c
extern void     initStartTime (void);
extern double   getTimeDiff (struct timeval *s, struct timeval *e);
extern double   getTotalTime (void);
extern void     printCurrentLocalTime();
extern void     initRand (void);
extern string   delUnnecessary (string & str);
extern string   getFirstWord (string & str);
extern void     showCoordinates (city_coords * coords, ROUTE_DATA_TYPE rangeStart,
                                 ROUTE_DATA_TYPE rangeEnd);
extern void     logToFile (char *filename, char *string);
extern void     printRoute (vector < ROUTE_DATA_TYPE > &route, city_coords * coords);
extern void     printRoute (vector < ROUTE_DATA_TYPE > &route,
                            ROUTE_DATA_TYPE rangeStart, ROUTE_DATA_TYPE rangeEnd,
                            city_coords * coords);
extern void     saveRouteToAFile (vector < ROUTE_DATA_TYPE > &route, char *filename,
                                  unsigned long length);
extern void     saveRouteToAFile (vector < ROUTE_DATA_TYPE > &route,
                                  ROUTE_DATA_TYPE rangeStart,
                                  ROUTE_DATA_TYPE rangeEnd, char *filename,
                                  unsigned long length);
extern void     saveRouteToAFile (vector < ROUTE_DATA_TYPE > &route, char *filename);
extern void     saveRouteToAFile (vector < ROUTE_DATA_TYPE > &route,
                                  ROUTE_DATA_TYPE rangeStart,
                                  ROUTE_DATA_TYPE rangeEnd, char *filename);
extern void     printRankNode (void);
extern void     printLicense (void);
extern void     printUsage (void);
extern string   getCPUInfo (void);

// solver.c
extern void     run (cmdArguments * args);
extern          vector < ROUTE_DATA_TYPE > optimize (cmdArguments * args,
        city_coords * coords,
        vector < ROUTE_DATA_TYPE > route,
        unsigned short method, int once =
            0);
extern void     benchmark (cmdArguments * args, city_coords * coords,
                           vector < ROUTE_DATA_TYPE > route, unsigned short method,
                           int BENCHMARK_ITERS);

#ifdef HAVE_PTHREADS
extern          vector < ROUTE_DATA_TYPE > optimizeThreaded (cmdArguments * args,
        city_coords * coords,
        vector <
        ROUTE_DATA_TYPE > route,
        unsigned short method);
extern void    *optimizationThread (void *data);
#endif

// construction.c
extern          vector < ROUTE_DATA_TYPE > multipleFragment (city_coords * coords,
        ROUTE_DATA_TYPE cities);
extern ROUTE_DATA_TYPE getNNSimple (ROUTE_DATA_TYPE index, city_coords * coords,
                                    ROUTE_DATA_TYPE cities);
extern void     routeInitFromFile (vector < ROUTE_DATA_TYPE > &route,
                                   ROUTE_DATA_TYPE size, char *filename);
extern void     routeInit (vector < ROUTE_DATA_TYPE > &route, int method,
                           ROUTE_DATA_TYPE size, city_coords * coords);
extern void     checkTour (vector < ROUTE_DATA_TYPE > route);

// kdtree.c
extern void     constructKDTree (city_coords * coords, ROUTE_DATA_TYPE cities);
extern          vector < ROUTE_DATA_TYPE > getNN_KD_list (ROUTE_DATA_TYPE index,
        city_coords * coords,
        ROUTE_DATA_TYPE howMany);
extern ROUTE_DATA_TYPE getNN_KD (ROUTE_DATA_TYPE index, city_coords * coords);
extern void     removePointsKDTree (city_coords * coords,
                                    vector < ROUTE_DATA_TYPE > &points);
extern void     removePointKD (city_coords * coords, ROUTE_DATA_TYPE index);
extern          vector < ROUTE_DATA_TYPE >
multipleFragment_KD (ROUTE_DATA_TYPE cities, city_coords * coords);
extern          vector < ROUTE_DATA_TYPE > initRoute_KD_NN (ROUTE_DATA_TYPE size,
        city_coords * coords);

// tsp_loader.c
extern int      readFileCoords (char *filename, city_coords * coords);

inline double
calculateDistance2D (ROUTE_DATA_TYPE i, ROUTE_DATA_TYPE j, city_coords * coords) {
    register float  dx,
             dy;
    dx = coords[i].x - coords[j].x;
    dy = coords[i].y - coords[j].y;
    return (sqrtf (dx * dx + dy * dy) + 0.5f);
};

inline double
calculateDistance2D (ROUTE_DATA_TYPE i, ROUTE_DATA_TYPE j, float *x, float *y) {
    register float  dx,
             dy;
    dx = x[i] - x[j];
    dy = y[i] - y[j];
    return (sqrtf (dx * dx + dy * dy) + 0.5f);
};

inline unsigned long
calculateSwapEffect (vector < ROUTE_DATA_TYPE > &route, ROUTE_DATA_TYPE a,
                     ROUTE_DATA_TYPE b, city_coords * coords) {
    if (a > 0 && b > 0)
        return calculateDistance2D (route[b], route[a], coords) +
               calculateDistance2D (route[a - 1], route[b - 1], coords) -
               calculateDistance2D (route[a - 1], route[a], coords) -
               calculateDistance2D (route[b - 1], route[b], coords);

    else if (a == 0 && b > 0)
        return calculateDistance2D (route[b], route[0], coords) +
               calculateDistance2D (route[route.size() - 1], route[b - 1], coords) -
               calculateDistance2D (route[route.size() - 1], route[0], coords) -
               calculateDistance2D (route[b - 1], route[b], coords);

    else if (a > 0 && b == 0)
        return calculateDistance2D (route[0], route[a], coords) +
               calculateDistance2D (route[a - 1], route[route.size() - 1], coords) -
               calculateDistance2D (route[a - 1], route[a], coords) -
               calculateDistance2D (route[route.size() - 1], route[0], coords);

    else {
        return 0;
    }
};

inline unsigned long
routeLength (vector < ROUTE_DATA_TYPE > &route, city_coords * coords,
             ROUTE_DATA_TYPE rangeStart, ROUTE_DATA_TYPE rangeEnd) {
    unsigned long   d = 0;

    for (ROUTE_DATA_TYPE i = rangeStart + 1; i < rangeEnd; i++) {
        d += calculateDistance2D (route[i - 1], route[i], coords);
    }

    if (rangeEnd == route.size() ) {
        d += calculateDistance2D (route[route.size() - 1], route[0], coords);
    }

    return d;
};

inline unsigned long
routeLength (vector < ROUTE_DATA_TYPE > &route, city_coords * coords) {
    return routeLength (route, coords, 0, route.size() );
};

#endif
