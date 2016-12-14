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

#include <cpuMTSolver.h>
#include <cpusimd.h>

void
CPUMTSolver::init() {
    // nothing?
};

void
CPUMTSolver::close() {
    // nothing?
};

struct process_time
CPUMTSolver::benchmark (vector < ROUTE_DATA_TYPE > &route) {

    struct best2_out out;
    struct timeval  start,
            end;
    process_time    time;

    gettimeofday (&start, NULL);

    out = optimizeStep (route);

    gettimeofday (&end, NULL);

    time.HtD_time = 0;
    time.kernel_time = getTimeDiff (&start, &end);
    time.DtH_time = 0;
    time.out = out;

    // debug code
    trace ("[%s] Local Optimization: 2-opt pair found (%d,%d) -> change %d\n",
           description.c_str(), out.i, out.j, out.minchange);

    return time;

};

// Implementation of the 2-opt exchange step

typedef struct {

    unsigned        threadID;
    vector < ROUTE_DATA_TYPE > tour;
    int             cities;
    city_coords    *coords;
#ifdef USE_ORDERED_COORDS
    float          *coordsX;
    float          *coordsY;
#endif
    cmdArguments   *args;
    struct best2_out *out;
    int             start_j;
    int             end_j;
    int             start_i;
    int             end_i;

} t_data_2opt;

struct best2_out
CPUMTSolver::optimizeStep (const vector < ROUTE_DATA_TYPE > &route) {

    struct best2_out out;
    out.minchange = 0;
#ifdef HAVE_PTHREADS

    int             ret;
    t_data_2opt     messages[MAX_THREADS];
    pthread_t       threads[MAX_THREADS];
    struct best2_out local_out[MAX_THREADS];

    long long       twoOpts = ( (route.size() - 2) * (route.size() - 3) ) / 2;
    long long       op_per_thread = twoOpts / args->maxCoresUsed;

#ifdef USE_ORDERED_COORDS
    // reorder coordinates in the route's order

    // create x and y float vectors for aligned coalesced SSE/AVX loads
    float          *coordsX = (float *) ALLOC (sizeof (float) * MAX_CITIES);
    float          *coordsY = (float *) ALLOC (sizeof (float) * MAX_CITIES);

    for (ROUTE_DATA_TYPE i = 0; i < route.size(); i++) {
        coordsX[i] = coords[route[i]].x;
        coordsY[i] = coords[route[i]].y;
#endif
    }

#endif

    for (int thread_no = 0; thread_no < args->maxCoresUsed; thread_no++) {
        long long       counter_start = thread_no * op_per_thread;
        long long       counter_end = (thread_no + 1) * op_per_thread - 1;
        // calculate indices for each thread
        unsigned int    start_i =
            int (3 + sqrt (8.0 * (long double) counter_start + 1.0) ) / 2.0;
        unsigned int    start_j =
            counter_start - (start_i - 2) * (start_i - 1) / 2.0 + 1;
        unsigned int    end_i =
            int (3 + sqrt (8.0 * (long double) counter_end + 1.0) ) / 2;
        unsigned int    end_j = counter_end - (end_i - 2) * (end_i - 1) / 2.0 + 1;
        messages[thread_no].threadID = thread_no;
#ifdef USE_ORDERED_COORDS
        messages[thread_no].coordsX = coordsX;
        messages[thread_no].coordsY = coordsY;
#else
        messages[thread_no].coords = coords;
        messages[thread_no].tour = route;
#endif
        messages[thread_no].args = args;
        // output
        messages[thread_no].out = &local_out[thread_no];
        // ranges
        messages[thread_no].start_j = start_j;
        messages[thread_no].end_j = end_j;
        messages[thread_no].start_i = start_i;
        messages[thread_no].end_i = end_i;
        pthread_create (&threads[thread_no], NULL, parallelCPU,
                        (void *) &messages[thread_no]);
    }

    for (int thread_no = 0; thread_no < args->maxCoresUsed; thread_no++) {
        pthread_join (threads[thread_no], NULL);

        if (out.minchange > local_out[thread_no].minchange) {
            out.minchange = messages[thread_no].out->minchange;
            out.i = messages[thread_no].out->i;
            out.j = messages[thread_no].out->j;
        }
    }

#ifdef USE_ORDERED_COORDS

    free (coordsX);
    free (coordsY);

#endif

    return out;
};

void           *
parallelCPU (void *data) {
    t_data_2opt     t = * (t_data_2opt *) data;
#ifdef __linux__

    if (t.args->setAffinity == 1) {
        cpu_set_t       mask;
        CPU_ZERO (&mask);
        CPU_SET (t.threadID % t.args->maxCoresUsed, &mask);

        if (sched_setaffinity (0, sizeof (mask), &mask) < 0) {
            perror ("sched_setaffinity");
        }
    }

#endif
    ROUTE_DATA_TYPE best_i,
                    best_j;
    int             minchange = INT_MAX;
    int             change;
#ifdef USE_CPU_SIMD
    VECF            dx1,
                    dy1,
                    dx2,
                    dy2,
                    dx3,
                    dy3,
                    dx4,
                    dy4,
                    sum1,
                    sum2,
                    diff;
    VECF            coordsB1X,
                    coordsB2X,
                    coordsB1Y,
                    coordsB2Y;
    const float     pointFive = 0.5f;
    VECF            constant = LOAD_CONST (pointFive);
    OUTPUT_VECTOR   o;
#endif

    for (int i = t.start_i; i < t.end_i; i++) {
#ifdef USE_ORDERED_COORDS
        register int    const1 = calculateDistance2D (i, i - 1, t.coordsX, t.coordsY);
#ifdef USE_CPU_SIMD
        VECF            coordsA2X = LOAD_CONST (t.coordsX[i - 1]);
        VECF            coordsA2Y = LOAD_CONST (t.coordsY[i - 1]);
        VECF            coordsA1Y = LOAD_CONST (t.coordsY[i]);
        VECF            coordsA1X = LOAD_CONST (t.coordsX[i]);
        dx3 = SUBF (coordsA1X, coordsA2X);
        dy3 = SUBF (coordsA1Y, coordsA2Y);
        dx3 = MULF (dx3, dx3);
        dy3 = MULF (dy3, dy3);
        dx3 = ADDF (dx3, dy3);
        dx3 = SQRTF (dx3);
#endif
#else
        register int    const1 =
            calculateDistance2D (t.tour[i], t.tour[i - 1], t.coords);
#endif
#if defined(USE_512BIT_VECTORS) && defined(USE_ALIGNED_LOADS_OPTIMIZATION)

        for (int j = 0; j < i - 1; j += VECTOR_LENGTH) {
#else

        for (int j = 1; j < i - 1; j += VECTOR_LENGTH) {
#endif
#ifndef USE_ORDERED_COORDS
            change =
                (unsigned
                 long) (calculateDistance2D (t.tour[i], t.tour[j + 1],
                                             t.coords) +
                        calculateDistance2D (t.tour[i - 1], t.tour[j], t.coords)
                        - const1 - calculateDistance2D (t.tour[j + 1], t.tour[j],
                                t.coords) - 0.5f);
#else
#ifndef USE_CPU_SIMD
            change =
                (unsigned long) (calculateDistance2D (i, j + 1, t.coordsX, t.coordsY)
                                 + calculateDistance2D (i - 1, j, t.coordsX,
                                         t.coordsY)
                                 - const1 - calculateDistance2D (j + 1, j, t.coordsX,
                                         t.coordsY) - 0.5f);

            if (change < 0 && change < minchange) {
                minchange = change;
                best_i = j + 1;
                best_j = i;
            }

#else

            // if more than VECTOR_LENGTH left...
            if (j < i - VECTOR_LENGTH - 1) {
#if defined(USE_512BIT_VECTORS) && defined(USE_ALIGNED_LOADS_OPTIMIZATION)
                coordsB2X = LOADA16F (& (t.coordsX[j]) );
                coordsB2Y = LOADA16F (& (t.coordsY[j]) );
                LOAD_UNALIGNED (coordsB1X, & (t.coordsX[j + 1]) );
                LOAD_UNALIGNED (coordsB1Y, & (t.coordsY[j + 1]) );
#else
                coordsB2X = LOADF (& (t.coordsX[j]) );
                coordsB2Y = LOADF (& (t.coordsY[j]) );
                coordsB1X = LOADF (& (t.coordsX[j + 1]) );
                coordsB1Y = LOADF (& (t.coordsY[j + 1]) );
#endif
                dx1 = SUBF (coordsA1X, coordsB1X);
                dy1 = SUBF (coordsA1Y, coordsB1Y);
                dx2 = SUBF (coordsA2X, coordsB2X);
                dy2 = SUBF (coordsA2Y, coordsB2Y);
                dx4 = SUBF (coordsB1X, coordsB2X);
                dy4 = SUBF (coordsB1Y, coordsB2Y);
                dx1 = MULF (dx1, dx1);
                dy1 = MULF (dy1, dy1);
                dx2 = MULF (dx2, dx2);
                dy2 = MULF (dy2, dy2);
                dx4 = MULF (dx4, dx4);
                dy4 = MULF (dy4, dy4);
                dx1 = ADDF (dx1, dy1);
                dx2 = ADDF (dx2, dy2);
                dx4 = ADDF (dx4, dy4);
                dx1 = SQRTF (dx1);
                dx2 = SQRTF (dx2);
                dx4 = SQRTF (dx4);
                sum1 = ADDF (dx1, dx2);
                sum2 = ADDF (dx3, dx4);
                diff = SUBF (sum1, sum2);
                diff = SUBF (diff, constant);
#ifdef USE_512BIT_VECTORS
                float           m = MIN16F (diff);

                if (m < minchange) {
#endif
                    o.v = diff;

                    for (int a = 0; a < VECTOR_LENGTH; a++) {
                        if (o.f[a] < minchange) {
                            minchange = o.f[a];
                            best_i = j + 1 + a;
                            best_j = i;
                        }
                    }

#ifdef USE_512BIT_VECTORS
                }

#endif

            } else {
                // if size(elements left) < VECTOR_LENGTH
                change =
                    (unsigned
                     long) (calculateDistance2D (i, j + 1, t.coordsX,
                                                 t.coordsY) + calculateDistance2D (i -
                                                         1,
                                                         j,
                                                         t.
                                                         coordsX,
                                                         t.
                                                         coordsY)
                            - const1 - calculateDistance2D (j + 1, j, t.coordsX,
                                    t.coordsY) - 0.5f);

                if (change < 0) {
                    if (change < minchange) {
                        minchange = change;
                        best_i = i;
                        best_j = j + 1;
                    }
                }
            }

#endif
#endif
        }
    }

    t.out->minchange = minchange;
    t.out->i = best_i;
    t.out->j = best_j;
    return 0;
}
