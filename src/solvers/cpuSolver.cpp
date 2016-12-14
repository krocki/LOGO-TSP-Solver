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

#include <cpuSolver.h>

void
CPUSolver::init() {
    // nothing?
};

void
CPUSolver::close() {
    // nothing?
};

struct process_time
CPUSolver::benchmark (vector < ROUTE_DATA_TYPE > &route) {

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

struct best2_out
CPUSolver::optimizeStep (const vector < ROUTE_DATA_TYPE > &route) {

    struct best2_out out;
    out.minchange = 0;
    int             temp_change;


#ifdef USE_ORDERED_COORDS
    // reorder coordinates in the route's order

    // create x and y float vectors for aligned coalesced SSE/AVX loads
    float          *coordsX = (float *) ALLOC (sizeof (float) * route.size() );
    float          *coordsY = (float *) ALLOC (sizeof (float) * route.size() );

    for (ROUTE_DATA_TYPE i = 0; i < route.size(); i++) {
        coordsX[i] = coords[route[i]].x;
        coordsY[i] = coords[route[i]].y;
    }

#endif

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

    for (ROUTE_DATA_TYPE i = 1; i < route.size() - 2; i++) {
#ifdef USE_ORDERED_COORDS
        register int    const1 = calculateDistance2D (i, i - 1, coordsX, coordsY);
#ifdef USE_CPU_SIMD
        VECF            coordsA2X = LOAD_CONST (coordsX[i - 1]);
        VECF            coordsA2Y = LOAD_CONST (coordsY[i - 1]);
        VECF            coordsA1Y = LOAD_CONST (coordsY[i]);
        VECF            coordsA1X = LOAD_CONST (coordsX[i]);
        dx3 = SUBF (coordsA1X, coordsA2X);
        dy3 = SUBF (coordsA1Y, coordsA2Y);
        dx3 = MULF (dx3, dx3);
        dy3 = MULF (dy3, dy3);
        dx3 = ADDF (dx3, dy3);
        dx3 = SQRTF (dx3);
#endif
#else
        register int    const1 = calculateDistance2D (route[i], route[i - 1], coords);
#endif

        for (ROUTE_DATA_TYPE j = i + 1; j < route.size() - 1; j += VECTOR_LENGTH) {
#ifndef USE_ORDERED_COORDS
            temp_change =
                (unsigned long) (calculateDistance2D (route[i], route[j + 1], coords)
                                 + calculateDistance2D (route[i - 1], route[j],
                                         coords) - const1 -
                                 calculateDistance2D (route[j + 1], route[j],
                                         coords) - 0.5f);
#else
#ifndef USE_CPU_SIMD
            temp_change =
                (unsigned long) (calculateDistance2D (i, j + 1, coordsX, coordsY) +
                                 calculateDistance2D (i - 1, j, coordsX,
                                         coordsY) - const1 -
                                 calculateDistance2D (j + 1, j, coordsX,
                                         coordsY) - 0.5f);

            if (temp_change < 0) {
                if (temp_change < out.minchange) {
                    out.minchange = temp_change;
                    out.i = i;
                    out.j = j + 1;
                }
            }

#else

            // if more than VECTOR_LENGTH left...
            if (j < route.size() - VECTOR_LENGTH) {
                coordsB1X = LOADF (coordsX + j + 1);
                coordsB2X = LOADF (coordsX + j);
                coordsB1Y = LOADF (coordsY + j + 1);
                coordsB2Y = LOADF (coordsY + j);
                dx1 = SUBF (coordsA1X, coordsB1X);
                dx1 = MULF (dx1, dx1);
                dy1 = SUBF (coordsA1Y, coordsB1Y);
                dy1 = MULF (dy1, dy1);
                dx1 = ADDF (dx1, dy1);
                dx1 = SQRTF (dx1);
                dx2 = SUBF (coordsA2X, coordsB2X);
                dx2 = MULF (dx2, dx2);
                dy2 = SUBF (coordsA2Y, coordsB2Y);
                dy2 = MULF (dy2, dy2);
                dx2 = ADDF (dx2, dy2);
                dx2 = SQRTF (dx2);
                sum1 = ADDF (dx1, dx2);
                dx4 = SUBF (coordsB1X, coordsB2X);
                dx4 = MULF (dx4, dx4);
                dy4 = SUBF (coordsB1Y, coordsB2Y);
                dy4 = MULF (dy4, dy4);
                dx4 = ADDF (dx4, dy4);
                dx4 = SQRTF (dx4);
                sum2 = ADDF (dx3, dx4);
                diff = SUBF (sum1, sum2);
                diff = SUBF (diff, constant);
#ifdef USE_512BIT_VECTORS
                float           m = MIN16F (diff);

                if (m < out.minchange) {
#endif
                    o.v = diff;

                    for (int a = 0; a < VECTOR_LENGTH; a++) {
                        if (o.f[a] < 0)
                            if (o.f[a] < out.minchange) {
                                out.minchange = o.f[a];
                                out.i = i;
                                out.j = j + 1 + a;
                            }
                    }

#ifdef USE_512BIT_VECTORS
                }

#endif

            } else {
                // if size(elements left) < VECTOR_LENGTH
                temp_change =
                    (unsigned long) (calculateDistance2D (i, j + 1, coordsX, coordsY)
                                     + calculateDistance2D (i - 1, j, coordsX,
                                             coordsY) - const1 -
                                     calculateDistance2D (j + 1, j, coordsX,
                                             coordsY) - 0.5f);

                if (temp_change < 0) {
                    if (temp_change < out.minchange) {
                        out.minchange = temp_change;
                        out.i = i;
                        out.j = j + 1;
                    }
                }
            }

#endif
#endif
        }
    }

#ifdef USE_ORDERED_COORDS
    free (coordsX);
    free (coordsY);
#endif
    return out;
};
