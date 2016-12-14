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

#ifndef _DEFS_CL_
#define _DEFS_CL_

#define CL_SAFE_CALL(x) ret = x; UtilsCL::checkError(ret, STR(x));

#define DEFAULT_CL_DEVICE 0
#define MAX_BLOCKS 8192
#define BUFFER_STRING_LENGTH 256

#define MAX_SOURCE_SIZE (0x100000)

#define CL_MAX_CITIES MAX_CITIES
#define CL_MAX_CITIES_SIMPLE 2048

typedef struct {

    cl_device_id    id;
    cl_platform_id  platform;
    cl_device_type  type;
    string          device_string;
    string          platform_string;
    cl_uint         compute_units;
    unsigned        workgroup_size;
    cl_ulong        global_mem_size;
    cl_ulong        local_mem_size;
    cl_uint         preferred_vector;

} cl_dev_info;

#endif
