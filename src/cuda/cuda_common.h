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

#ifndef LEGACY_GPUS
// mutex lock
inline __device__ void
lock (int *mutex) {
    while (atomicCAS (mutex, 0, 1) != 0);
}

// mutex unlock
inline __device__ void
unlock (int *mutex) {
    atomicExch (mutex, 0);
}
#endif

// distance function for the simple kernel
inline __device__ int
calculateDistance2DSimple (unsigned int i, unsigned int j, city_coords * coords) {
    register float  dx,
             dy;
    dx = coords[i].x - coords[j].x;
    dy = coords[i].y - coords[j].y;
    return (int) (sqrtf (dx * dx + dy * dy) + 0.5f);
}

// distance function for the extended kernel
inline __device__ int
calculateDistance2D_extended (unsigned int i, unsigned int j, city_coords * coordsA,
                              city_coords * coordsB) {
    register float  dx,
             dy;
    dx = coordsA[i].x - coordsB[j].x;
    dy = coordsA[i].y - coordsB[j].y;
    return (int) (sqrtf (dx * dx + dy * dy) + 0.5f);
}
