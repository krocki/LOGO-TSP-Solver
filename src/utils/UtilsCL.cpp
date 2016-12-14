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
#include <UtilsCL.h>

std::vector < cl_dev_info > UtilsCL::getAllDevices (void) {
    cl_int
    ret;
    vector < cl_dev_info > available_devices;
    cl_device_id   *
    devices;
    cl_uint
    ret_num_devices;
    cl_platform_id *
    platforms;
    cl_uint
    ret_num_platforms;
    // trace("Thread %d, Looking for OpenCL platforms...\n", threadID);
    // get the OpenCL platforms' number
    CL_SAFE_CALL (clGetPlatformIDs (0, NULL, &ret_num_platforms) );
    // trace("Thread %d, %d OpenCL platform(s) detected.\n", threadID,
    // ret_num_platforms);
    platforms =
        (cl_platform_id *) malloc (sizeof (cl_platform_id) * ret_num_platforms);

    if (!platforms) {
        // printf("Thread %d, Couldn't allocate memory for CL platforms", threadID);
        exit (-1);
    }

    // get the OpenCL platforms
    CL_SAFE_CALL (clGetPlatformIDs (ret_num_platforms, platforms, &ret_num_platforms) );
    // temp vars
    char
    name[BUFFER_STRING_LENGTH];
    char
    vendor[BUFFER_STRING_LENGTH];
    char
    version[BUFFER_STRING_LENGTH];
    char
    profile[BUFFER_STRING_LENGTH];

    for (unsigned i = 0; i < ret_num_platforms; i++) {
        CL_SAFE_CALL (clGetPlatformInfo
                      (platforms[i], CL_PLATFORM_VENDOR, BUFFER_STRING_LENGTH, vendor,
                       NULL) );
        CL_SAFE_CALL (clGetPlatformInfo
                      (platforms[i], CL_PLATFORM_NAME, BUFFER_STRING_LENGTH, name,
                       NULL) );
        CL_SAFE_CALL (clGetPlatformInfo
                      (platforms[i], CL_PLATFORM_VERSION, BUFFER_STRING_LENGTH,
                       version, NULL) );
        CL_SAFE_CALL (clGetPlatformInfo
                      (platforms[i], CL_PLATFORM_PROFILE, BUFFER_STRING_LENGTH,
                       profile, NULL) );
        clGetDeviceIDs (platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &ret_num_devices);

        if (ret_num_devices == 0) {
            trace ("No devices found supporting OpenCL.\n");
            exit (-1);

        } else {
            devices =
                (cl_device_id *) malloc (sizeof (cl_device_id) * ret_num_devices);
            CL_SAFE_CALL (clGetDeviceIDs
                          (platforms[i], CL_DEVICE_TYPE_ALL, ret_num_devices, devices,
                           &ret_num_devices) );

            for (unsigned j = 0; j < ret_num_devices; ++j) {
                cl_dev_info
                found_device;
                found_device = getDevice (devices[j]);
                found_device.id = devices[j];
                found_device.platform = platforms[i];
                std::string temp = std::string (name);
                found_device.platform_string = delUnnecessary (temp);
                available_devices.push_back (found_device);
            }

            free (devices);
        }
    }

    free (platforms);
    return available_devices;
}

std::vector < deviceInfo > UtilsCL::listDevices (void) {
    vector < deviceInfo > devices;
    vector < cl_dev_info > available_devices = getAllDevices();
    deviceInfo
    temp;

    for (unsigned i = 0; i < available_devices.size(); i++) {
        std::stringstream sstm;
        sstm << available_devices[i].device_string << " (" << available_devices[i].
             compute_units << " Compute Units, " << (float) available_devices[i].
             global_mem_size / (1024.0f * 1024.0f) << " MB)";
        temp.name = sstm.str();
        temp.localNum = (long) available_devices[i].id;
        temp.type =
            getFirstWord (available_devices[i].platform_string) + string (" OpenCL");
        temp.typeId = TYPE_GLOBAL_SOLVER_CL;
        devices.push_back (temp);
    }

    return devices;
}

cl_dev_info
UtilsCL::getDevice (cl_device_id device) {
    cl_dev_info     found_device;
    cl_int          ret;
    char            device_string[BUFFER_STRING_LENGTH];
    cl_uint         compute_units;
    size_t          workgroup_size;
    cl_ulong        global_mem_size;
    cl_ulong        local_mem_size;
    cl_uint         preferred_vector;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_NAME, sizeof (device_string), &device_string,
                   NULL) );
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof (compute_units),
                   &compute_units, NULL) );
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof (workgroup_size),
                   &workgroup_size, NULL) );
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof (global_mem_size),
                   &global_mem_size, NULL) );
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof (local_mem_size),
                   &local_mem_size, NULL) );
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof (cl_uint),
                   &preferred_vector, NULL) );
    std::string temp = std::string (device_string);
    found_device.device_string = delUnnecessary (temp);
    found_device.compute_units = compute_units;
    found_device.workgroup_size = (unsigned) workgroup_size;
    found_device.global_mem_size = global_mem_size;
    found_device.local_mem_size = local_mem_size;
    found_device.preferred_vector = preferred_vector;
    return found_device;
}

void
UtilsCL::devInfo (cl_device_id device, int extended) {
    char            device_string[BUFFER_STRING_LENGTH];
    cl_bool         buf_bool;
    cl_int          ret;
    // CL_DEVICE_NAME
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_NAME, sizeof (device_string), &device_string,
                   NULL) );
    printf ("\t\t\tCL_DEVICE_NAME = %s\n", device_string);
    // CL_DEVICE_VENDOR
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_VENDOR, sizeof (device_string), &device_string,
                   NULL) );
    printf ("\t\t\tCL_DEVICE_VENDOR = %s\n", device_string);
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_VERSION, sizeof (device_string), &device_string,
                   NULL) );
    printf ("\t\t\tCL_DEVICE_VERSION = %s\n", device_string);
    // CL_DRIVER_VERSION
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DRIVER_VERSION, sizeof (device_string), &device_string,
                   NULL) );
    printf ("\t\t\tCL_DRIVER_VERSION = %s\n", device_string);
    // CL_DEVICE_INFO
    cl_device_type  type;
    CL_SAFE_CALL (clGetDeviceInfo (device, CL_DEVICE_TYPE, sizeof (type), &type, NULL) );

    if (type & CL_DEVICE_TYPE_CPU) {
        printf ("\t\t\tCL_DEVICE_TYPE = CL_DEVICE_TYPE_CPU\n");
    }

    if (type & CL_DEVICE_TYPE_GPU) {
        printf ("\t\t\tCL_DEVICE_TYPE = CL_DEVICE_TYPE_GPU\n");
    }

    if (type & CL_DEVICE_TYPE_ACCELERATOR) {
        printf ("\t\t\tCL_DEVICE_TYPE = CL_DEVICE_TYPE_ACCELERATOR\n");
    }

    if (type & CL_DEVICE_TYPE_DEFAULT) {
        printf ("\t\t\tCL_DEVICE_TYPE = CL_DEVICE_TYPE_DEFAULT\n");
    }

    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_AVAILABLE, sizeof (buf_bool), &buf_bool, NULL) );

    if (extended) {
        printf ("\t\t\tCL_DEVICE_AVAILABLE = %s\n",
                buf_bool == CL_TRUE ? "YES" : "NO");
    }

    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_COMPILER_AVAILABLE, sizeof (buf_bool), &buf_bool,
                   NULL) );

    if (extended) {
        printf ("\t\t\tCL_DEVICE_COMPILER_AVAILABLE = %s\n",
                buf_bool == CL_TRUE ? "YES" : "NO");
    }

    // CL_DEVICE_MAX_COMPUTE_UNITS
    cl_uint         compute_units;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof (compute_units),
                   &compute_units, NULL) );
    printf ("\t\t\tCL_DEVICE_MAX_COMPUTE_UNITS = %u\n", compute_units);
    // CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS
    size_t          workitem_dims;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof (workitem_dims),
                   &workitem_dims, NULL) );
    printf ("\t\t\tCL_DEVICE_MAX_WORK_ITEM_DIMENSIONS = %u\n",
            (unsigned) workitem_dims);
    // CL_DEVICE_MAX_WORK_ITEM_SIZES
    size_t          workitem_size[3];
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof (workitem_size),
                   &workitem_size, NULL) );
    printf ("\t\t\tCL_DEVICE_MAX_WORK_ITEM_SIZES = %u / %u / %u \n",
            (unsigned) workitem_size[0], (unsigned) workitem_size[1],
            (unsigned) workitem_size[2]);
    // CL_DEVICE_MAX_WORK_GROUP_SIZE
    size_t          workgroup_size;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof (workgroup_size),
                   &workgroup_size, NULL) );
    printf ("\t\t\tCL_DEVICE_MAX_WORK_GROUP_SIZE = %u\n", (unsigned) workgroup_size);
    // CL_DEVICE_MAX_CLOCK_FREQUENCY
    cl_uint         clock_frequency;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof (clock_frequency),
                   &clock_frequency, NULL) );
    printf ("\t\t\tCL_DEVICE_MAX_CLOCK_FREQUENCY = %u MHz\n", clock_frequency);
    // CL_DEVICE_ADDRESS_BITS
    cl_uint         addr_bits;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_ADDRESS_BITS, sizeof (addr_bits), &addr_bits,
                   NULL) );

    if (extended) {
        printf ("\t\t\tCL_DEVICE_ADDRESS_BITS = %u\n", addr_bits);
    }

    // CL_DEVICE_MAX_MEM_ALLOC_SIZE
    cl_ulong        max_mem_alloc_size;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof (max_mem_alloc_size),
                   &max_mem_alloc_size, NULL) );

    if (extended) {
        printf ("\t\t\tCL_DEVICE_MAX_MEM_ALLOC_SIZE = %u MB\n",
                (unsigned int) (max_mem_alloc_size / (1024 * 1024) ) );
    }

    // CL_DEVICE_GLOBAL_MEM_SIZE
    cl_ulong        mem_size;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof (mem_size), &mem_size,
                   NULL) );
    printf ("\t\t\tCL_DEVICE_GLOBAL_MEM_SIZE = %u MB\n",
            (unsigned int) (mem_size / (1024 * 1024) ) );
    // TODO
    // CL_DEVICE_GLOBAL_MEM_CACHE_SIZE
    // CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE
    // CL_DEVICE_ERROR_CORRECTION_SUPPORT
    cl_bool         error_correction_support;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_ERROR_CORRECTION_SUPPORT,
                   sizeof (error_correction_support), &error_correction_support,
                   NULL) );
    printf ("\t\t\tCL_DEVICE_ERROR_CORRECTION_SUPPORT = %s\n",
            error_correction_support == CL_TRUE ? "YES" : "NO");
    // CL_DEVICE_LOCAL_MEM_TYPE
    cl_device_local_mem_type local_mem_type;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof (local_mem_type),
                   &local_mem_type, NULL) );

    if (extended) {
        printf ("\t\t\tCL_DEVICE_LOCAL_MEM_TYPE = %s\n",
                local_mem_type == 1 ? "LOCAL" : "GLOBAL");
    }

    // CL_DEVICE_LOCAL_MEM_SIZE
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof (mem_size), &mem_size,
                   NULL) );
    printf ("\t\t\tCL_DEVICE_LOCAL_MEM_SIZE = %u kB\n",
            (unsigned int) (mem_size / 1024) );
    // CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof (mem_size),
                   &mem_size, NULL) );
    printf ("\t\t\tCL_DEVICE_MAX_CONSTANT_BUFFER_SIZE = %u kB\n",
            (unsigned int) (mem_size / 1024) );
    // CL_DEVICE_QUEUE_PROPERTIES
    cl_command_queue_properties queue_properties;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_QUEUE_PROPERTIES, sizeof (queue_properties),
                   &queue_properties, NULL) );

    if (extended)
        if (queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) {
            printf
            ("\t\t\tCL_DEVICE_QUEUE_PROPERTIES = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE\n");
        }

    if (extended)
        if (queue_properties & CL_QUEUE_PROFILING_ENABLE) {
            printf ("\t\t\tCL_DEVICE_QUEUE_PROPERTIES = CL_QUEUE_PROFILING_ENABLE\n");
        }

    // CL_DEVICE_IMAGE_SUPPORT
    cl_bool         image_support;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_IMAGE_SUPPORT, sizeof (image_support),
                   &image_support, NULL) );

    if (extended) {
        printf ("\t\t\tCL_DEVICE_IMAGE_SUPPORT = %u\n", image_support);
    }

    // CL_DEVICE_MAX_READ_IMAGE_ARGS
    cl_uint         max_read_image_args;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof (max_read_image_args),
                   &max_read_image_args, NULL) );

    if (extended) {
        printf ("\t\t\tCL_DEVICE_MAX_READ_IMAGE_ARG = %u\n", max_read_image_args);
    }

    // CL_DEVICE_MAX_WRITE_IMAGE_ARGS
    cl_uint         max_write_image_args;
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS,
                   sizeof (max_write_image_args), &max_write_image_args, NULL) );

    if (extended) {
        printf ("\t\t\tCL_DEVICE_MAX_WRITE_IMAGE_ARGS = %u\n", max_write_image_args);
    }

    // CL_DEVICE_IMAGE2D_MAX_WIDTH, CL_DEVICE_IMAGE2D_MAX_HEIGHT,
    // CL_DEVICE_IMAGE3D_MAX_WIDTH, CL_DEVICE_IMAGE3D_MAX_HEIGHT,
    // CL_DEVICE_IMAGE3D_MAX_DEPTH
    size_t          szMaxDims[5];

    if (extended) {
        printf ("\t\t\tCL_DEVICE_IMAGE:\n");
    }

    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof (size_t), &szMaxDims[0],
                   NULL) );

    if (extended) {
        printf ("\t\t\t\t2D_MAX_WIDTH = %u\n", (unsigned) szMaxDims[0]);
    }

    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof (size_t),
                   &szMaxDims[1], NULL) );

    if (extended) {
        printf ("\t\t\t\t2D_MAX_HEIGHT = %u\n", (unsigned) szMaxDims[1]);
    }

    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof (size_t), &szMaxDims[2],
                   NULL) );

    if (extended) {
        printf ("\t\t\t\t3D_MAX_WIDTH = %u\n", (unsigned) szMaxDims[2]);
    }

    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof (size_t),
                   &szMaxDims[3], NULL) );

    if (extended) {
        printf ("\t\t\t\t3D_MAX_HEIGHT = %u\n", (unsigned) szMaxDims[3]);
    }

    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof (size_t), &szMaxDims[4],
                   NULL) );

    if (extended) {
        printf ("\t\t\t\t3D_MAX_DEPTH = %u\n", (unsigned) szMaxDims[4]);
    }

    // CL_DEVICE_EXTENSIONS: get device extensions, and if any then parse & log the
    // string onto separate lines
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_EXTENSIONS, sizeof (device_string),
                   &device_string, NULL) );

    if (device_string != 0) {
        char            delimiter[] = " ";
        int             inputLength = strlen (device_string);
        char           *word,
                       *context;
        char           *inputCopy = (char *) calloc (inputLength + 1, sizeof (char) );
        strncpy (inputCopy, device_string, inputLength);

        if (extended) {
            printf ("\t\t\tCL_DEVICE_EXTENSIONS:\n");
        }

        word = strtok_r (inputCopy, delimiter, &context);

        if (extended) {
            printf ("\t\t\t\t%s\n", word);
        }

        while (1) {
            word = strtok_r (NULL, delimiter, &context);

            if (word == NULL) {
                break;
            }

            if (extended) {
                printf ("\t\t\t\t%s\n", word);
            }
        }

        free (inputCopy);

    } else {
        if (extended) {
            printf ("\t\t\tCL_DEVICE_EXTENSIONS: NONE\n");
        }
    }

    // CL_DEVICE_PREFERRED_VECTOR_WIDTH_<type>
    if (extended) {
        printf ("\t\t\tCL_DEVICE_PREFERRED_VECTOR_WIDTH:\t\n");
    }

    cl_uint         vec_width[6];
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof (cl_uint),
                   &vec_width[0], NULL) );
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof (cl_uint),
                   &vec_width[1], NULL) );
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof (cl_uint),
                   &vec_width[2], NULL) );
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof (cl_uint),
                   &vec_width[3], NULL) );
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof (cl_uint),
                   &vec_width[4], NULL) );
    CL_SAFE_CALL (clGetDeviceInfo
                  (device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof (cl_uint),
                   &vec_width[5], NULL) );

    if (extended)
        printf
        ("\t\t\t\tCHAR = %u\n\t\t\t\tSHORT = %u\n\t\t\t\tINT = %u\n\t\t\t\tFLOAT = %u\n\t\t\t\tDOUBLE = %u\n",
         vec_width[0], vec_width[1], vec_width[2], vec_width[3], vec_width[4]);
}

const char     *
UtilsCL::oclErrorString (cl_int error) {
    static const char *errorstring[] = {
        "cl_success",
        "cl_device_not_found",
        "cl_device_not_available",
        "cl_compiler_not_available",
        "cl_mem_object_allocation_failure",
        "cl_out_of_resources",
        "cl_out_of_host_memory",
        "cl_profiling_info_not_available",
        "cl_mem_copy_overlap",
        "cl_image_format_mismatch",
        "cl_image_format_not_supported",
        "cl_build_program_failure",
        "cl_map_failure",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "cl_invalid_value",
        "cl_invalid_device_type",
        "cl_invalid_platform",
        "cl_invalid_device",
        "cl_invalid_context",
        "cl_invalid_queue_properties",
        "cl_invalid_command_queue",
        "cl_invalid_host_ptr",
        "cl_invalid_mem_object",
        "cl_invalid_image_format_descriptor",
        "cl_invalid_image_size",
        "cl_invalid_sampler",
        "cl_invalid_binary",
        "cl_invalid_build_options",
        "cl_invalid_program",
        "cl_invalid_program_executable",
        "cl_invalid_kernel_name",
        "cl_invalid_kernel_definition",
        "cl_invalid_kernel",
        "cl_invalid_arg_index",
        "cl_invalid_arg_value",
        "cl_invalid_arg_size",
        "cl_invalid_kernel_args",
        "cl_invalid_work_dimension",
        "cl_invalid_work_group_size",
        "cl_invalid_work_item_size",
        "cl_invalid_global_offset",
        "cl_invalid_event_wait_list",
        "cl_invalid_event",
        "cl_invalid_operation",
        "cl_invalid_gl_object",
        "cl_invalid_buffer_size",
        "cl_invalid_mip_level",
        "cl_invalid_global_work_size",
    };
    const int       errorcount = sizeof (errorstring) / sizeof (errorstring[0]);
    const int       index = -error;
    return (index >= 0 && index < errorcount) ? errorstring[index] : "";
}

void
UtilsCL::checkError (const cl_int ciErrNum, const char *const operation) {
    if (ciErrNum != CL_SUCCESS) {
        printf ("ERROR: %s failed, %s\n", operation, oclErrorString (ciErrNum) );
        // exit(EXIT_FAILURE);
    }
}
