/*
 *   Logo TSP Solver ver. 0.62  Copyright (C) 2013  Kamil Rocki
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it wzill be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DEFS_H_
#include <vector>

// data type used to store cities' IDs (should be small, but sufficient, i.e.
// unsigned int)
#define ROUTE_DATA_TYPE unsigned int

// max number of cities handled - memory limited
#define MAX_CITIES 2097152

// PI - needed in case of GEO coordinates
#define PI 3.14159265358979323846264f

#define USE_ORDERED_COORDS

#ifdef __linux__
// use aligned malloc
#define USE_MEM_ALIGN
#define MEM_ALIGN 1024
#endif

#ifdef USE_MEM_ALIGN
#define ALLOC(size) memalign( MEM_ALIGN, size);
#else
#define ALLOC(size) malloc( size);
#endif

// approximation of sqrt(AVX's sqrt is a bottleneck (split into 128-bit operations))
// // used in cpusimd.h
// #ifdef USE_AVX
#define USE_FAST_SQRT
// #endif

#ifdef USE_FAST_SQRT
#define CPU_FLOPS_PER_DISTANCE 7
#else
#define CPU_FLOPS_PER_DISTANCE 6
#endif

// some faster bitwise operations
#define swapint(x,y) { x ^= y; y ^= x; x ^= y;}
#define negative(x) (~x + 1)
#define mod256(x) (x & 255)
#define mod128(x) (x & 127)
#define mod64(x) (x & 63)
#define mod32(x) (x & 31)
#define mod16(x) (x & 15)
#define mod8(x) (x & 7)
#define mod4(x) (x & 3)
#define mod2(x) (x & 1)

#define DEFAULT_FILENAME "./TSPLIB/berlin52.tsp"
#define DEFAULT_OUT "out"
#define DEFAULT_INIT "mf"
#define DEFAULT_BACKTRACKING "NO"
#define DEFAULT_BACKTRACKING_LIMIT 2
#define DEFAULT_GPU_THREADS 1024
#define DEFAULT_GPU_BLOCKS 16
#define DEFAULT_TIMELIMIT 0
#define DEFAULT_ERROR_TOLERANCE 0.005f
#define DEFAULT_PTHREADS 1
#define DEFAULT_AFFINITY 0
#define DEFAULT_PTHREADS_COMM 1
#define DEFAULT_PTHREADS_COMM_PART 1.0f
#define DEFAULT_STALL_ITERS 500
#define DEFAULT_STALL_TIME_PERIOD 3
#define DEFAULT_STALL_TIME_MULTIPLIER 30
#define DEFAULT_TRACK_SOLUTION 0

#define NO_BACKTRACKING 0
#define ITER_BACKTRACKING 1
#define TIME_BACKTRACKING 2

#define TYPE_GLOBAL_SOLVER_CPU 0
#define TYPE_GLOBAL_SOLVER_CUDA 1
#define TYPE_GLOBAL_SOLVER_CL 2
#define TYPE_GLOBAL_SOLVER_CPU_PARALLEL 3
#define TYPE_GLOBAL_SOLVER_CPU_PARALLEL_HYBRID 4

#define DEFAULT_GLOBAL_SOLVER TYPE_GLOBAL_SOLVER_CPU

// log
#define TIME printCurrentLocalTime();
#define trace TIME printf

// log to a file
#define INIT_TRACEF char temp_string[256];
#define tracef(X, ...)  sprintf(temp_string, X, ##__VA_ARGS__); \
						logToFile(args->outfilename, temp_string)

#define MAX_THREADS 512

// route init methods
#define ROUTE_INIT_SIMPLE 0
#define ROUTE_INIT_NN 1
#define ROUTE_INIT_SHUFFLE 2
#define ROUTE_INIT_MF 3

#define START_NUMBERING_FROM 0

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

#define MODE_CPU 0
#define MODE_CUDA 1
#define MODE_OPENCL 2
#define MODE_CPU_PARALLEL 3
#define MODE_CPU_PARALLEL_HYBRID 4
#define MODE_ALL 5
#define MODE_CPU_ALL 6
#define DEFAULT_MODE MODE_CPU

#define DEFAULT_SHOW_LO_INFO 0

#endif

// used structures
#ifndef _STRUCTS
#define _STRUCTS

// input parameters
typedef struct {

    char           *filename;
    char           *outfilename;
    short           gpuThreads;
    short           gpuBlocks;
    unsigned long   solution;
    float           timelimit;
    float           error;
    short           initRouteFromFile;
    char           *initRouteFile;
    char           *initMethod;
    short           mode;
    ROUTE_DATA_TYPE maxKicks;
    short           device;
    short           maxDevices;
    short           autoDevice;
    unsigned        pthreads;
    unsigned        setAffinity;
    unsigned        maxCoresUsed;
    char           *backtracking;
    unsigned        backtrackingLimit;
    short           comm;
    float           commPart;
    unsigned        stallIterations;
    unsigned        stallTimePeriod;
    unsigned        stallTimeMultiplier;
    short           showLocalOptimizationInfo;
    short           trackSolution;
    short           benchmark;
    short           vectorsize;

} cmdArguments;

// coordinate's storage
typedef struct cc {

    float           x;
    float           y;

} city_coords;

// best 2-opt exchange
struct best2_out {

    ROUTE_DATA_TYPE i;
    ROUTE_DATA_TYPE j;
    int             minchange;

};

// processing time and estimated computation performed (FLOPs)
struct process_time {

    double          kernel_time;	// kernel time
    double          HtD_time;	// Host to device transfer time
    // (equals 0 if CPU processing)
    double          DtH_time;	// Device to host transfer time
    // (equals 0 if CPU processing)
    struct best2_out out;
};

// multiple fragment heuristic - kdtree_utils.c
typedef struct fragment {

    int             distance;
    ROUTE_DATA_TYPE id;

} fragment;

class           CompareFragments {
public:
    bool operator() (fragment & f1, fragment & f2) {
    	// Returns true if t1 is
        // earlier than t2
        if (f1.distance > f2.distance) {
            return true;
        }

        return          false;
    }
};

typedef struct {

    unsigned        id;
    std::vector < ROUTE_DATA_TYPE > route;
    cmdArguments   *args;
    city_coords    *coords;
    unsigned short  method;
    unsigned        dev;

} threadData;

typedef struct {

    std::string name;
    std::string type;
    short           typeId;
    std::string subtype;
    long            localNum;
    int             globalNum;

} deviceInfo;

#endif
