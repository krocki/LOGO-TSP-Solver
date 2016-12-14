for a in `seq 1000`
do

	./logo --in="./TSPLIB/sw24978.tsp" --initMethod="mf" --mode="cpupar" --maxCoresCPU=32 --err=0 --timelimit=86400 --showLOInfo=1 --device=0 --comm=1 --out="sw24978_E5_2690_CPUPAR_32th_v062"
	./logo --in="./TSPLIB/sw24978.tsp" --initMethod="mf" --mode="cpuhybrid" --maxCoresCPU=32 --err=0 --timelimit=86400 --showLOInfo=1 --device=0 --comm=1 --out="sw24978_E5_2690_CPUHYBRID_COMM_32th_v062"
	./logo --in="./TSPLIB/sw24978.tsp" --initMethod="mf" --mode="cpuhybrid" --maxCoresCPU=32 --err=0 --timelimit=86400 --showLOInfo=1 --device=0 --comm=0 --out="sw24978_E5_2690_CPUHYBRID_NOCOMM_32th_v062"
	./logo --in="./TSPLIB/sw24978.tsp" --initMethod="mf" --mode="cpu" --maxCoresCPU=1 --err=0 --timelimit=86400 --showLOInfo=1 --device=0 --comm=1 --out="sw24978_E5_2690_AVX_seq_v062"

done
