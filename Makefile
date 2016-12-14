all:		logo

clean:
		rm -rf ./out/* ./interim/* logo

GCC_PATH = gcc
CXX_PATH = g++
INTEL_COMPILER_PATH = icpc

#set 0 or 1
HAVE_CUDA = 0
HAVE_OPENCL = 0
HAVE_PTHREADS = 1

USE_INTEL_COMPILER = 0

#SSE/AVX enabled
USE_CPU_SIMD = 1
USE_AVX = 1
USE_MIC = 0

SSE_VERSION = -msse4.2

VECTORIZER_VERBOSE = 0

#support for NVIDIA GPUs older than Fermi
LEGACY_GPUS = 0

ifeq ($(USE_INTEL_COMPILER), 1)
	CC := $(INTEL_COMPILER_PATH)
else
	CC := $(CXX_PATH)
endif

NPROCS := 1
OS := $(shell uname)

ifeq ($(OS),Linux)
  NPROCS := $(shell grep -c ^processor /proc/cpuinfo)
else ifeq ($(OS),Darwin)
  NPROCS := $(shell sysctl hw.ncpu | awk '{print $$2}')
endif # $(OS)

HOST := $(shell hostname | awk -F. '{print $$1}')

ifeq ($(OS),Linux)
#LINUX
CUDA_LIBRARY_DIR = /usr/local/cuda-5.0/lib64
CUDA_INCLUDE_DIR = /usr/local/cuda-5.0/include
NVCC_PATH = /usr/local/cuda-5.0/bin/nvcc
OPENCL_LIBRARY_DIR =  /opt/intel/opencl/lib64
OPENCL_INCLUDE_DIR =  /opt/intel/opencl/include/
#OPENCL_LIBRARY_DIR = /usr/local/cuda/lib/
#OPENCL_INCLUDE_DIR = /usr/local/cuda/include/
endif
ifeq ($(OS),Darwin)
#MACOS - mine at least
CUDA_LIBRARY_DIR = /usr/local/cuda/lib
CUDA_INCLUDE_DIR = /Developer/NVIDIA/CUDA-5.0/include
NVCC_PATH = /Developer/NVIDIA/CUDA-5.0/bin/nvcc
OPENCL_LIBRARY_DIR =  /usr/local/cuda/lib
OPENCL_INCLUDE_DIR =  /Developer/NVIDIA/CUDA-5.0/include
endif

OUT_FILES = $(wildcard ./out/*.o)

#limit the number of regs, i.e. for Fermi GPUs
CUDA_LIMIT_REGISTER_NO = 0

CUDA_L1_CACHING = 1

ifeq ($(CUDA_L1_CACHING), 0)
	CUDA_CACHING = -Xptxas -dlcm=cg
else
    CUDA_CACHING = -Xptxas -dlcm=ca
endif

OPENCLFLAGS = -Wall -O3

CFLAGS = -O3 -ftree-vectorizer-verbose=$(VECTORIZER_VERBOSE) -march=native -mtune=native  -Wall -D_CRT_SECURE_NO_DEPRECATE -fpermissive
INCLUDE_DIRS = -I./src/solvers/ -I./src/utils/ -I./src/ -I./src/parser/ -I./src/cuda/ -I./src/cl/ -I./src/kdtree++/ -I$(OPENCL_INCLUDE_DIR)
CUDA_ARCHS = -gencode arch=compute_20,code=sm_20 -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35

CUDA_OPTS = -fmad=true -prec-div=false -ftz=true -prec-sqrt=false --use_fast_math

NVCCFLAGS = -O3 $(CUDA_CACHING)

ifeq ($(CUDA_LIMIT_REGISTER_NO), 1)
	CUDA_OPTS := $(CUDA_OPTS) --maxrregcount=32
endif

ifeq ($(LEGACY_GPUS), 1)
	CUDA_ARCHS := $(CUDA_ARCHS) -gencode arch=compute_10,code=sm_10 -gencode arch=compute_11,code=sm_11 -gencode arch=compute_12,code=sm_12 -gencode arch=compute_13,code=sm_13
	CFLAGS := $(CFLAGS) -DLEGACY_GPUS
	NVCCFLAGS := $(NVCCFLAGS) -DLEGACY_GPUS
	CUDA_OPTS := $(CUDA_OPTS) --maxrregcount=16
endif

ALL_FILES = info ./out/main.o ./out/parser.o ./out/cut.o ./out/common.o ./out/ILSGlobalSolverSequential.o ./out/ILSGlobalSolverMT.o ./out/ILSGlobalSolver.o ./out/cpuSolver.o ./out/cpuMTSolver.o ./out/twoOptLocalSolver.o ./out/solver.o ./out/tsp_loader.o ./out/construction.o ./out/kdtree_utils.o ./out/cutil.o

ifeq ($(USE_INTEL_COMPILER), 1)
#fast math, less precise
	CFLAGS := $(CFLAGS) -fimf-precision=low -fimf-domain-exclusion=15

	ifeq ($(USE_MIC), 1)
		CFLAGS := $(CFLAGS) -mmic -DUSE_MIC -opt-prefetch -opt-prefetch-distance=32,4 -ansi-alias

	else
		CFLAGS := $(CFLAGS) -xhost -xavx
	endif

else

endif

ifeq ($(USE_CPU_SIMD),1)
	CFLAGS := $(CFLAGS) -DUSE_CPU_SIMD

	ifeq ($(USE_AVX),1)
		CFLAGS := $(CFLAGS) -DUSE_AVX
	endif

ifeq ($(USE_INTEL_COMPILER), 0)
	ifeq ($(USE_AVX),1)
		CFLAGS := $(CFLAGS) -mavx
	else
		CFLAGS := $(CFLAGS) $(SSE_VERSION)
	endif
endif

endif



ifeq ($(OS),Linux)
	CL_LINK_FLAGS = -L$(OPENCL_LIBRARY_DIR) -lOpenCL
	CUDA_LINK_FLAGS = -lcuda -lcudart
	ARCH = -m64

endif
ifeq ($(OS),Darwin)
	CL_LINK_FLAGS = -framework OpenCL
	CUDA_LINK_FLAGS = -lcuda -lcudart
	ARCH = -m32
endif

	CFLAGS := $(CFLAGS) $(ARCH)
	NVCCCFLAGS := $(NVCCCFLAGS) $(ARCH)
	OPENCLFLAGS := $(OPENCLFLAGS) $(ARCH)

ifeq ($(HAVE_OPENCL), 1)
	CFLAGS := $(CFLAGS) -DHAVE_OPENCL
	LINKFLAGS := $(LINKFLAGS) $(CL_LINK_FLAGS)
	ALL_FILES := $(ALL_FILES) ./out/CLSolver.o ./out/UtilsCL.o
endif

ifeq ($(HAVE_CUDA), 1)
	CFLAGS := $(CFLAGS) -DHAVE_CUDA
	NVCCFLAGS := $(NVCCFLAGS) -DHAVE_CUDA
	LINKFLAGS := $(LINKFLAGS) -L$(CUDA_LIBRARY_DIR) -I$(CUDA_INCLUDE_DIR) $(CUDA_LINK_FLAGS)
	ALL_FILES := $(ALL_FILES) ./out/opt_kernel.o ./out/CUDASolver.o ./out/UtilsCUDA.o
endif

ifeq ($(HAVE_PTHREADS), 1)
	CFLAGS := $(CFLAGS) -DHAVE_PTHREADS
	NVCCFLAGS := $(NVCCFLAGS) -DHAVE_PTHREADS
	LINKFLAGS := $(LINKFLAGS) -lpthread
endif


#NVCCFLAGS := $(NVCCFLAGS) --keep --keep-dir interim
NVCCFLAGS := $(NVCCFLAGS) --ptxas-options=-v $(CUDA_ARCHS) $(CUDA_LINK_FLAGS) $(CUDA_OPTS)


CFLAGS := $(CFLAGS) -DDEFAULT_MAX_CORES=$(NPROCS) -DHOST=$(HOST) $(INCLUDE_DIRS)
NVCCFLAGS := $(NVCCFLAGS) -DDEFAULT_MAX_CORES=$(NPROCS) -DHOST=$(HOST) $(INCLUDE_DIRS)


info:
		@echo
		@echo "HOST =" $(HOST)
		@echo "HAVE_OPENCL =" $(HAVE_OPENCL)
		@echo "HAVE_CUDA =" $(HAVE_CUDA)
		@echo "CUDA_L1_CACHING =" $(CUDA_L1_CACHING)
		@echo "HAVE_PTHREADS =" $(HAVE_PTHREADS)
		@echo "CPU_SIMD =" $(CPU_SIMD)
		@echo "DETECTED OS =" $(OS)
		@echo "DETECTED" $(NPROCS) "CPUs"
		@echo "OPENCLFLAGS =" $(OPENCLFLAGS)
		@echo "CFLAGS =" $(CFLAGS)
		@echo "CUDA_ARCHS =" $(CUDA_ARCHS)
		@echo "CUDA_OPTS =" $(CUDA_OPTS)
		@echo

logo:           	$(ALL_FILES)
	    $(CC) $(CFLAGS) -o logo $(OUT_FILES) $(LINKFLAGS)

./out/main.o:           ./src/main.c ./src/headers.h ./src/defs.h
		$(CC) $(CFLAGS) -o ./out/main.o -c ./src/main.c

./out/solver.o:         ./src/solver.c ./src/headers.h ./src/defs.h ./src/solvers/*.h ./src/solvers/*.cpp
		$(CC) $(CFLAGS) -o ./out/solver.o -c ./src/solver.c

./out/common.o:         ./src/utils/common.c ./src/headers.h ./src/defs.h
		$(CC) $(CFLAGS) -o ./out/common.o -c ./src/utils/common.c

./out/cut.o:            ./src/utils/cut.c ./src/headers.h ./src/defs.h
		$(CC) $(CFLAGS) -o ./out/cut.o -c ./src/utils/cut.c

./out/tsp_loader.o:     ./src/utils/tsp_loader.c ./src/headers.h ./src/defs.h
		$(CC) $(CFLAGS) -o ./out/tsp_loader.o -c ./src/utils/tsp_loader.c

./out/cutil.o:          ./src/parser/cutil.cpp ./src/parser/cutil.h
		$(CC) $(CFLAGS) -o ./out/cutil.o -c ./src/parser/cutil.cpp

./out/parser.o:         ./src/parser/cmd_arg_reader.cpp ./src/parser/cmd_arg_reader.h
		$(CC) $(CFLAGS) -o ./out/parser.o -c ./src/parser/cmd_arg_reader.cpp

./out/construction.o:   ./src/utils/construction.c ./src/headers.h ./src/defs.h
		$(CC) $(CFLAGS) -o ./out/construction.o -c ./src/utils/construction.c

./out/kdtree_utils.o:   ./src/utils/kdtree_utils.c ./src/utils/kdtree_utils.h ./src/headers.h ./src/defs.h
		$(CC) $(CFLAGS) -o ./out/kdtree_utils.o -c ./src/utils/kdtree_utils.c

./out/cpuSolver.o:		./src/solvers/cpuSolver.cpp ./src/solvers/cpuSolver.h ./src/solvers/twoOptLocalSolver.cpp ./src/solvers/twoOptLocalSolver.h ./src/solvers/localSolver.h ./src/headers.h ./src/defs.h ./src/utils/cpusimd.h
		$(CC) $(CFLAGS) -o ./out/cpuSolver.o -c ./src/solvers/cpuSolver.cpp

./out/cpuMTSolver.o:		./src/solvers/cpuMTSolver.cpp ./src/solvers/cpuMTSolver.h ./src/solvers/twoOptLocalSolver.cpp ./src/solvers/twoOptLocalSolver.h ./src/solvers/localSolver.h ./src/headers.h ./src/defs.h ./src/utils/cpusimd.h
		$(CC) $(CFLAGS) -o ./out/cpuMTSolver.o -c ./src/solvers/cpuMTSolver.cpp

./out/twoOptLocalSolver.o:	./src/solvers/twoOptLocalSolver.cpp ./src/solvers/twoOptLocalSolver.h ./src/solvers/localSolver.h ./src/headers.h ./src/defs.h
		$(CC) $(CFLAGS) -o ./out/twoOptLocalSolver.o -c ./src/solvers/twoOptLocalSolver.cpp

./out/ILSGlobalSolverSequential.o:	./src/solvers/ILSGlobalSolverSequential.cpp ./src/solvers/ILSGlobalSolverSequential.h ./src/solvers/ILSGlobalSolver.cpp ./src/solvers/ILSGlobalSolver.h ./src/headers.h ./src/defs.h
		$(CC) $(CFLAGS) -o ./out/ILSGlobalSolverSequential.o -c ./src/solvers/ILSGlobalSolverSequential.cpp

./out/ILSGlobalSolverMT.o:	./src/solvers/ILSGlobalSolverMT.cpp ./src/solvers/ILSGlobalSolverMT.h ./src/solvers/ILSGlobalSolver.cpp ./src/solvers/ILSGlobalSolver.h ./src/headers.h ./src/defs.h
		$(CC) $(CFLAGS) -o ./out/ILSGlobalSolverMT.o -c ./src/solvers/ILSGlobalSolverMT.cpp

./out/ILSGlobalSolver.o:	./src/solvers/ILSGlobalSolver.cpp ./src/solvers/ILSGlobalSolver.h ./src/headers.h ./src/defs.h ./src/solvers/localSolver.h
		$(CC) $(CFLAGS) -o ./out/ILSGlobalSolver.o -c ./src/solvers/ILSGlobalSolver.cpp

#######CUDA
./out/opt_kernel.o:     ./src/cuda/opt_kernel.cu ./src/headers.h ./src/defs.h ./src/cuda/cuda_common.h ./src/cuda/cuda_defs.h
		$(NVCC_PATH) $(NVCCFLAGS) -L$(CUDA_BIN_DIR) -I$(CUDA_INCLUDE_DIR) -o ./out/opt_kernel.o -c ./src/cuda/opt_kernel.cu

./out/CUDASolver.o:		./src/solvers/CUDASolver.cpp ./src/solvers/CUDASolver.h ./src/solvers/twoOptLocalSolver.cpp ./src/solvers/twoOptLocalSolver.h ./src/solvers/localSolver.h ./src/headers.h ./src/defs.h ./src/cuda/cuda_common.h ./src/cuda/cuda_defs.h
		$(NVCC_PATH) -x cu $(NVCCFLAGS) -L$(CUDA_LIBRARY_DIR) -L$(CUDA_BIN_DIR) -I$(CUDA_INCLUDE_DIR) -o ./out/CUDASolver.o -c ./src/solvers/CUDASolver.cpp

./out/UtilsCUDA.o:		./src/utils/UtilsCUDA.cpp ./src/utils/UtilsCUDA.h ./src/headers.h ./src/defs.h ./src/cuda/cuda_defs.h
		$(NVCC_PATH) -x cu $(NVCCFLAGS) -L$(CUDA_LIBRARY_DIR) -L$(CUDA_BIN_DIR) -I$(CUDA_INCLUDE_DIR) -o ./out/UtilsCUDA.o -c ./src/utils/UtilsCUDA.cpp

#######

#######OpenCL

./out/CLSolver.o:		./src/solvers/CLSolver.cpp ./src/solvers/CLSolver.h ./src/solvers/twoOptLocalSolver.cpp ./src/solvers/twoOptLocalSolver.h ./src/solvers/localSolver.h ./src/headers.h ./src/defs.h ./src/cl/cl_defs.h
		$(CC) -L$(OPENCL_LIBRARY_DIR) -I$(OPENCL_INCLUDE_DIR) $(OPENCLFLAGS) -DHAVE_OPENCL $(INCLUDE_DIRS) -Wall -ansi -g -O3 -o ./out/CLSolver.o -c ./src/solvers/CLSolver.cpp $(CL_LINK_FLAGS)

./out/UtilsCL.o:       	./src/utils/UtilsCL.cpp ./src/utils/UtilsCL.h ./src/headers.h ./src/defs.h ./src/cl/cl_defs.h
		$(CC) -L$(OPENCL_LIBRARY_DIR) -I$(OPENCL_INCLUDE_DIR) $(OPENCLFLAGS) -DHAVE_OPENCL $(INCLUDE_DIRS) -Wall -ansi -g -O3 -o ./out/UtilsCL.o -c ./src/utils/UtilsCL.cpp $(CL_LINK_FLAGS)
#######

