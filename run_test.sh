
#some sample modes

#modify Makefile for CUDA, OpenCL, pthreads support

#cpu
./logo --in="./TSPLIB/vm1084.tsp" --initMethod="mf" --mode="cpu" --device=0 --showLOInfo=1
./logo --in="./TSPLIB/vm1084.tsp" --initMethod="mf" --mode="cpu" --device=0 --showLOInfo=1 --out="vm1084_output"
#cpu multithreaded, default num of cores
./logo --in="./TSPLIB/vm1084.tsp" --initMethod="mf" --mode="cpumt" --device=0
#cpu multithreaded, use 8 cores, enable inter-thread communication
./logo --in="./TSPLIB/vm1084.tsp" --initMethod="mf" --mode="cpumt" --device=0 --comm=1 --maxCoresCPU=8
#cpu multithreaded, use 8 cores, assign threads to cores, enable inter-thread communication
./logo --in="./TSPLIB/vm1084.tsp" --initMethod="mf" --mode="cpumt" --device=0 --maxCoresCPU=8 --comm=1 --setAffinity=1
#really long - all of the above + log to a file (vm1084_cpu_E5645_x12_comm_aff.txt) including local optimization
./logo --in="./TSPLIB/vm1084.tsp" --initMethod="mf" --mode="cpumt" --device=0 --maxCoresCPU=12 --comm=1 --setAffinity=1 --showLOInfo=1 --out="vm1084_cpu_E5645_x12_comm_aff" 
#cuda, interactive mode
./logo --in="./TSPLIB/vm1084.tsp" --initMethod="mf" --mode="cuda"
#cuda, interactive mode, show local optimization info for multiple threads
./logo --in="./TSPLIB/vm1084.tsp" --initMethod="mf" --mode="cuda" --showLOInfo=1
#cuda, interactive mode, show local optimization info for multiple threads, enable inter-thread communication
./logo --in="./TSPLIB/vm1084.tsp" --initMethod="mf" --mode="cuda" --showLOInfo=1 --comm=1
#cuda, select a device a priori (0)
./logo --in="./TSPLIB/vm1084.tsp" --initMethod="mf" --mode="cuda" --device=0
#all types of devices, interactive mode, communication enabled
./logo --in="./TSPLIB/fnl4461.tsp" --initMethod="mf" --mode="all" --comm=1
#opencl, use all devices (autodevice assignes threads to each device), communication enabled
./logo --in="./TSPLIB/usa13509.tsp" --initMethod="mf" --mode="cl" --comm=1 --autoDevice

