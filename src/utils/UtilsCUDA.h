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
#include <cuda_defs.h>
#include <defs.h>

#ifdef __NVCC__
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_defs.h>
#endif

#ifndef _UTILS_CUDA_
#define _UTILS_CUDA_

#ifdef HAVE_CUDA

class           UtilsCUDA {

public:

    static          std::vector < cuda_dev_info > getCUDAInfo (int dev, int extended);
    static          std::vector < deviceInfo > listDevices (void);
    static void     CudaTest (const char *msg);
};
#endif
#endif
