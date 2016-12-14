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

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <headers.h>
#include <cl_defs.h>

#ifndef _UTILS_CL_
#define _UTILS_CL_

#ifdef HAVE_OPENCL

class           UtilsCL {

public:

    static          std::vector < cl_dev_info > getAllDevices();
    static          std::vector < deviceInfo > listDevices();
    static cl_dev_info getDevice (cl_device_id device);
    static void     devInfo (cl_device_id device, int extended);
    static void     checkError (const cl_int ciErrNum, const char *const operation);
    static const char *oclErrorString (cl_int error);

};
#endif
#endif
