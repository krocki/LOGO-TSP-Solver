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
#include <UtilsCUDA.h>


vector < deviceInfo > UtilsCUDA::listDevices (void) {
    vector < deviceInfo > devices;
    vector < cuda_dev_info > available_devices = getCUDAInfo (-1, 0);
    deviceInfo
    temp;

    for (unsigned i = 0; i < available_devices.size(); i++) {
        std::stringstream sstm;
        sstm << available_devices[i].device_string << " (" << available_devices[i].
             MPs << " x " << available_devices[i].
             coresPerMP << " Cores @ " << available_devices[i].
             clockRate << " GHz, " << available_devices[i].global_mem_size << " MB)";
        temp.name = sstm.str();
        temp.localNum = i;
        temp.type = string ("CUDA");
        temp.typeId = TYPE_GLOBAL_SOLVER_CUDA;
        devices.push_back (temp);
    }

    return devices;
}

/*
 * The original license, code has been modified to run within Logo TSP Solver
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

/*
 * This sample queries the properties of the CUDA devices present in the system via
 * CUDA Runtime API.
 */

// from CUDA SDK's deviceQuery.cpp

template < class T > inline void
getCudaAttribute (T * attribute, CUdevice_attribute device_attribute, int device) {
    CUresult
    error = cuDeviceGetAttribute (attribute, device_attribute, device);

    if (CUDA_SUCCESS != error) {
        fprintf (stderr,
                 "cuSafeCallNoSync() Driver API error = %04d from file <%s>, line %i.\n",
                 error, __FILE__, __LINE__);
        exit (-1);
    }
}

// Beginning of CUDA Architecture definitions
inline int
ConvertSMVer2Cores (int major, int minor) {
    // Defines for CUDA Architecture types (using the SM version to determine the #
    // of cores per SM
    typedef struct {
        int
        SM;			// 0xMm (hexidecimal notation), M = SM Major
        // version, and m = SM minor version
        int
        Cores;
    } sSMtoCores;
    sSMtoCores
    nGpuArchCoresPerSM[] = {
        {0x10, 8},
        {0x11, 8},
        {0x12, 8},
        {0x13, 8},
        {0x20, 32},
        {0x21, 48},
        {0x30, 192},
        {0x35, 192},
        { -1, -1}
    };
    int
    index = 0;

    while (nGpuArchCoresPerSM[index].SM != -1) {
        if (nGpuArchCoresPerSM[index].SM == ( (major << 4) + minor) ) {
            return nGpuArchCoresPerSM[index].Cores;
        }

        index++;
    }

    printf ("MapSMtoCores undefined SMversion %d.%d!\n", major, minor);
    return -1;
}
// end of CUDA Architecture definitions

vector < cuda_dev_info > UtilsCUDA::getCUDAInfo (int dev, int extended) {
    int
    deviceCount = 0;
    vector < cuda_dev_info > cudaDevices;
    cudaError_t
    error_id = cudaGetDeviceCount (&deviceCount);

    if (error_id != cudaSuccess) {
        trace ("cudaGetDeviceCount returned %d\n-> %s\n", (int) error_id,
               cudaGetErrorString (error_id) );
    }

    if (deviceCount == 0) {
        // trace("There is no device supporting CUDA\n");
        return cudaDevices;

    } else {
        // trace("Found %d CUDA Capable device(s)\n", deviceCount);
        if (dev >= deviceCount && dev != -1) {
            trace ("Wrong device number!\n");
            return cudaDevices;
        }
    }

    int
    driverVersion = 0,
    runtimeVersion = 0;

    if (dev == -1) {
        for (dev = 0; dev < deviceCount; dev++) {
            cuda_dev_info
            temp;
            cudaSetDevice (dev);
            cudaDeviceProp
            deviceProp;
            cudaGetDeviceProperties (&deviceProp, dev);
            temp.id = dev;
            std::string tempString = std::string (deviceProp.name);
            temp.device_string = delUnnecessary (tempString);
#if CUDART_VERSION >= 2000
            temp.MPs = deviceProp.multiProcessorCount;
            temp.coresPerMP = ConvertSMVer2Cores (deviceProp.major, deviceProp.minor);
#else
            temp.MPs = 0;
            temp.coresPerMP = 0;
#endif
            temp.global_mem_size = (float) deviceProp.totalGlobalMem / 1048576.0f;
            temp.local_mem_size = (unsigned) deviceProp.sharedMemPerBlock;
            temp.clockRate = deviceProp.clockRate * 1e-6f;
            cudaDevices.push_back (temp);
        }

    } else {
        cudaSetDevice (dev);
        cudaDeviceProp
        deviceProp;
        cudaGetDeviceProperties (&deviceProp, dev);
        trace ("Device %d: \"%s\"\n", dev, deviceProp.name);

        if (extended) {
#if CUDART_VERSION >= 2020
            // Console log
            cudaDriverGetVersion (&driverVersion);
            cudaRuntimeGetVersion (&runtimeVersion);
            trace ("  CUDA Driver Version / Runtime Version          %d.%d / %d.%d\n",
                   driverVersion / 1000, (driverVersion % 100) / 10,
                   runtimeVersion / 1000, (runtimeVersion % 100) / 10);
#endif
            trace ("  CUDA Capability Major/Minor version number:    %d.%d\n",
                   deviceProp.major, deviceProp.minor);
            trace
            ("  Total amount of global memory:                 %.0f MBytes (%llu bytes)\n",
             (float) deviceProp.totalGlobalMem / 1048576.0f,
             (unsigned long long) deviceProp.totalGlobalMem);
#if CUDART_VERSION >= 2000
            trace
            ("  (%2d) Multiprocessors x (%3d) CUDA Cores/MP:    %d CUDA Cores\n",
             deviceProp.multiProcessorCount, ConvertSMVer2Cores (deviceProp.major,
                     deviceProp.
                     minor),
             ConvertSMVer2Cores (deviceProp.major,
                                 deviceProp.minor) *
             deviceProp.multiProcessorCount);
#endif
            trace
            ("  CUDA Clock rate:                                %.0f MHz (%0.2f GHz)\n",
             deviceProp.clockRate * 1e-3f, deviceProp.clockRate * 1e-6f);
#if CUDART_VERSION >= 4000
            // This is not available in the CUDA Runtime API, so we make the
            // necessary calls the driver API to support this for output
            int
            memoryClock;
            getCudaAttribute < int > (&memoryClock,
                                      CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE, dev);
            trace ("  Memory Clock rate:                             %.0f Mhz\n",
                   memoryClock * 1e-3f);
            int
            memBusWidth;
            getCudaAttribute < int > (&memBusWidth,
                                      CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH,
                                      dev);
            trace ("  Memory Bus Width:                              %d-bit\n",
                   memBusWidth);
            int
            L2CacheSize;
            getCudaAttribute < int > (&L2CacheSize, CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE,
                                      dev);

            if (L2CacheSize) {
                trace ("  L2 Cache Size:                                 %d bytes\n",
                       L2CacheSize);
            }

            trace
            ("  Max Texture Dimension Size (x,y,z)             1D=(%d), 2D=(%d,%d), 3D=(%d,%d,%d)\n",
             deviceProp.maxTexture1D, deviceProp.maxTexture2D[0],
             deviceProp.maxTexture2D[1], deviceProp.maxTexture3D[0],
             deviceProp.maxTexture3D[1], deviceProp.maxTexture3D[2]);
            trace
            ("  Max Layered Texture Size (dim) x layers        1D=(%d) x %d, 2D=(%d,%d) x %d\n",
             deviceProp.maxTexture1DLayered[0],
             deviceProp.maxTexture1DLayered[1],
             deviceProp.maxTexture2DLayered[0],
             deviceProp.maxTexture2DLayered[1],
             deviceProp.maxTexture2DLayered[2]);
#endif
            trace ("  Total amount of constant memory:               %u bytes\n",
                   (unsigned) deviceProp.totalConstMem);
            trace ("  Total amount of shared memory per block:       %u bytes\n",
                   (unsigned) deviceProp.sharedMemPerBlock);
            trace ("  Total number of registers available per block: %d\n",
                   deviceProp.regsPerBlock);
            trace ("  Warp size:                                     %d\n",
                   deviceProp.warpSize);
            trace ("  Maximum number of threads per multiprocessor:  %d\n",
                   deviceProp.maxThreadsPerMultiProcessor);
            trace ("  Maximum number of threads per block:           %d\n",
                   deviceProp.maxThreadsPerBlock);
            trace ("  Maximum sizes of each dimension of a block:    %d x %d x %d\n",
                   deviceProp.maxThreadsDim[0], deviceProp.maxThreadsDim[1],
                   deviceProp.maxThreadsDim[2]);
            trace ("  Maximum sizes of each dimension of a grid:     %d x %d x %d\n",
                   deviceProp.maxGridSize[0], deviceProp.maxGridSize[1],
                   deviceProp.maxGridSize[2]);
            trace ("  Maximum memory pitch:                          %u bytes\n",
                   (unsigned) deviceProp.memPitch);
            trace ("  Texture alignment:                             %u bytes\n",
                   (unsigned) deviceProp.textureAlignment);
#if CUDART_VERSION >= 4000
            trace
            ("  Concurrent copy and execution:                 %s with %d copy engine(s)\n",
             (deviceProp.deviceOverlap ? "Yes" : "No"),
             deviceProp.asyncEngineCount);
#else
            trace ("  Concurrent copy and execution:                 %s\n",
                   deviceProp.deviceOverlap ? "Yes" : "No");
#endif
#if CUDART_VERSION >= 2020
            trace ("  Run time limit on kernels:                     %s\n",
                   deviceProp.kernelExecTimeoutEnabled ? "Yes" : "No");
            trace ("  Integrated CUDA sharing Host Memory:            %s\n",
                   deviceProp.integrated ? "Yes" : "No");
            trace ("  Support host page-locked memory mapping:       %s\n",
                   deviceProp.canMapHostMemory ? "Yes" : "No");
#endif
#if CUDART_VERSION >= 3000
            trace ("  Concurrent kernel execution:                   %s\n",
                   deviceProp.concurrentKernels ? "Yes" : "No");
            trace ("  Alignment requirement for Surfaces:            %s\n",
                   deviceProp.surfaceAlignment ? "Yes" : "No");
#endif
#if CUDART_VERSION >= 3010
            trace ("  Device has ECC support enabled:                %s\n",
                   deviceProp.ECCEnabled ? "Yes" : "No");
#endif
#if CUDART_VERSION >= 3020
            trace ("  Device is using TCC driver mode:               %s\n",
                   deviceProp.tccDriver ? "Yes" : "No");
#endif
#if CUDART_VERSION >= 4000
            trace ("  Device supports Unified Addressing (UVA):      %s\n",
                   deviceProp.unifiedAddressing ? "Yes" : "No");
            trace ("  Device PCI Bus ID / PCI location ID:           %d / %d\n",
                   deviceProp.pciBusID, deviceProp.pciDeviceID);
#endif
#if CUDART_VERSION >= 2020
            const char     *
            sComputeMode[] = {
                "Default (multiple host threads can use ::cudaSetDevice() with device simultaneously)",
                "Exclusive (only one host thread in one process is able to use ::cudaSetDevice() with this device)",
                "Prohibited (no host thread can use ::cudaSetDevice() with this device)",
                "Exclusive Process (many threads in one process is able to use ::cudaSetDevice() with this device)",
                "Unknown",
                NULL
            };
            trace ("  Compute Mode:\n");
            trace ("     < %s >\n", sComputeMode[deviceProp.computeMode]);
#endif
        }
    }

    return cudaDevices;
}

void
UtilsCUDA::CudaTest (const char *msg) {
    cudaError_t     e;
    cudaThreadSynchronize();

    if (cudaSuccess != (e = cudaGetLastError() ) ) {
        fprintf (stderr, "%s: %d\n", msg, e);
        fprintf (stderr, "%s\n", cudaGetErrorString (e) );
        exit (-1);
    }
}
