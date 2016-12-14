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

#include <cutil.h>
#include <headers.h>

cmdArguments
parseInput(int argc, const char **argv)
{
    cmdArguments    args;
    char           *inputArg = NULL;
    trace("----------\n");
    trace("Setting parameters...\n");
    // for the default values see defs.h
    args.filename = (char *) DEFAULT_FILENAME;
    args.outfilename = (char *) DEFAULT_OUT;
    args.gpuThreads = DEFAULT_GPU_THREADS;
    args.gpuBlocks = DEFAULT_GPU_BLOCKS;
    args.pthreads = DEFAULT_PTHREADS;
    args.setAffinity = DEFAULT_AFFINITY;
    args.timelimit = DEFAULT_TIMELIMIT;
    args.error = DEFAULT_ERROR_TOLERANCE;
    args.initRouteFromFile = 0;
    args.initMethod = (char *) DEFAULT_INIT;
    args.mode = DEFAULT_MODE;
    args.device = -1;
    args.autoDevice = 0;
    args.maxKicks = 0;
    args.stallIterations = DEFAULT_STALL_ITERS;
    args.backtracking = (char *) DEFAULT_BACKTRACKING;
    args.backtrackingLimit = DEFAULT_BACKTRACKING_LIMIT;
    args.stallIterations = DEFAULT_STALL_ITERS;
    args.stallTimePeriod = DEFAULT_STALL_TIME_PERIOD;
    args.stallTimeMultiplier = DEFAULT_STALL_TIME_MULTIPLIER;
    args.showLocalOptimizationInfo = DEFAULT_SHOW_LO_INFO;
    args.comm = DEFAULT_PTHREADS_COMM;
    args.commPart = DEFAULT_PTHREADS_COMM_PART;
    args.trackSolution = DEFAULT_TRACK_SOLUTION;
    args.benchmark = 0;
    args.maxDevices = -1;
#ifdef DEFAULT_MAX_CORES
    args.maxCoresUsed = DEFAULT_MAX_CORES;
#else
    args.maxCoresUsed = 32;
#endif

    // number of cpu threads to be used (if using parallel cpu computation)
    if (cutGetCmdLineArgumentstr(argc, argv, "pthreads", &inputArg)) {
	args.pthreads = atoi(inputArg);
    }

    if (cutGetCmdLineArgumentstr(argc, argv, "setAffinity", &inputArg)) {
	args.setAffinity = atoi(inputArg);
    }

    if (cutGetCmdLineArgumentstr(argc, argv, "maxCoresCPU", &inputArg)) {
	args.maxCoresUsed = atoi(inputArg);
    }

    if (cutGetCmdLineArgumentstr(argc, argv, "cpuvector", &inputArg)) {
	args.vectorsize = atoi(inputArg);
    }
    // file for writing output data
    if (cutGetCmdLineArgumentstr(argc, argv, "out", &inputArg)) {
	args.outfilename = inputArg;
    }
    // input (TSPLIB format) file
    if (cutGetCmdLineArgumentstr(argc, argv, "in", &inputArg)) {
	args.filename = (char *) inputArg;
    }
    // CUDA blocks
    if (cutGetCmdLineArgumentstr(argc, argv, "blocks", &inputArg)) {
	args.gpuBlocks = atoi(inputArg);
    }
    // CUDA threads per block
    if (cutGetCmdLineArgumentstr(argc, argv, "threads", &inputArg)) {
	args.gpuThreads = atoi(inputArg);
    }
    // Time limit (if used)
    if (cutGetCmdLineArgumentstr(argc, argv, "timelimit", &inputArg)) {
	args.timelimit = atof(inputArg);
    }
    // Acceptable distance to the optimal (or best known) solution
    if (cutGetCmdLineArgumentstr(argc, argv, "err", &inputArg)) {
	args.error = atof(inputArg);
    }
    // init from a saved tour
    if (cutGetCmdLineArgumentstr(argc, argv, "initRouteFromFile", &inputArg)) {
	args.initRouteFromFile = atoi(inputArg);
    }
    // tour to be loaded
    if (cutGetCmdLineArgumentstr(argc, argv, "initRouteFile", &inputArg)) {
	args.initRouteFile = (char *) inputArg;
    }
    // tour construction method
    if (cutGetCmdLineArgumentstr(argc, argv, "initMethod", &inputArg)) {
	args.initMethod = (char *) inputArg;
    }
    // mode
    if (cutGetCmdLineArgumentstr(argc, argv, "mode", &inputArg)) {
	if (!strcmp((char *) inputArg, "cpu")) {
	    args.mode = MODE_CPU;

	} else if (!strcmp((char *) inputArg, "cuda")) {
	    args.mode = MODE_CUDA;

	} else if (!strcmp((char *) inputArg, "cl")) {
	    args.mode = MODE_OPENCL;

	} else if (!strcmp((char *) inputArg, "all")) {
	    args.mode = MODE_ALL;

	} else if (!strcmp((char *) inputArg, "cpumt")) {
	    args.mode = MODE_CPU_ALL;

	} else if (!strcmp((char *) inputArg, "cpupar")) {
	    args.mode = MODE_CPU_PARALLEL;

	} else if (!strcmp((char *) inputArg, "cpuhybrid")) {
	    args.mode = MODE_CPU_PARALLEL_HYBRID;
	}
    }

    if (cutGetCmdLineArgumentstr(argc, argv, "benchmark", &inputArg)) {
	args.benchmark = atoi(inputArg);
    }

    if (cutGetCmdLineArgumentstr(argc, argv, "device", &inputArg)) {
	args.device = atoi(inputArg);
    }

    if (cutGetCmdLineArgumentstr(argc, argv, "autoDevice", &inputArg)) {
	args.autoDevice = 1;
    }

    if (cutGetCmdLineArgumentstr(argc, argv, "maxDevices", &inputArg)) {
	args.maxDevices = atoi(inputArg);
    }
    // perturbation, no of kicks
    if (cutGetCmdLineArgumentstr(argc, argv, "maxKicks", &inputArg)) {
	args.maxKicks = atoi(inputArg);
    }
    // backtracking, type
    if (cutGetCmdLineArgumentstr(argc, argv, "backtracking", &inputArg)) {
	args.backtracking = (char *) (inputArg);
    }
    // backtracking, every n iterations
    if (cutGetCmdLineArgumentstr(argc, argv, "stallIterations", &inputArg)) {
	args.stallIterations = atoi(inputArg);
    }
    // backtracking, max moves back divider
    if (cutGetCmdLineArgumentstr(argc, argv, "backtrackingLimit", &inputArg)) {
	args.backtrackingLimit = atoi(inputArg);
    }
    // backtracking, time (last values considered)
    if (cutGetCmdLineArgumentstr(argc, argv, "stallTimePeriod", &inputArg)) {
	args.stallTimePeriod = atoi(inputArg);
    }
    // backtracking, time (multiplier)
    if (cutGetCmdLineArgumentstr(argc, argv, "stallTimeMultiplier", &inputArg)) {
	args.stallTimeMultiplier = atoi(inputArg);
    }
    // local optimization messages
    if (cutGetCmdLineArgumentstr(argc, argv, "showLOInfo", &inputArg)) {
	args.showLocalOptimizationInfo = atoi(inputArg);
    }
    // inter-thread communication
    if (cutGetCmdLineArgumentstr(argc, argv, "comm", &inputArg)) {
	args.comm = atoi(inputArg);
    }
    // define part of the threads to communicate
    if (cutGetCmdLineArgumentstr(argc, argv, "commPart", &inputArg)) {
	args.commPart = atof(inputArg);
    }

    if (cutGetCmdLineArgumentstr(argc, argv, "trackSolution", &inputArg)) {
	args.trackSolution = atoi(inputArg);
    }

    if (args.commPart < 0.0f) {
	args.commPart = 0.0f;
    }

    if (args.commPart > 1.0f) {
	args.commPart = 1.0f;
    }

    if (args.error < 0.0f) {
	args.error = 0.0f;
    }
    // some known solutions to compare with during the computation
    if (!strcmp(args.filename, "./TSPLIB/berlin52.tsp")) {
	args.solution = 7542;

    } else if (!strcmp(args.filename, "./TSPLIB/ch130.tsp")) {
	args.solution = 6110;

    } else if (!strcmp(args.filename, "./TSPLIB/pr439.tsp")) {
	args.solution = 107217;

    } else if (!strcmp(args.filename, "./TSPLIB/kroA100.tsp")) {
	args.solution = 21282;

    } else if (!strcmp(args.filename, "./TSPLIB/kroE100.tsp")) {
	args.solution = 22068;

    } else if (!strcmp(args.filename, "./TSPLIB/kroB100.tsp")) {
	args.solution = 22141;

    } else if (!strcmp(args.filename, "./TSPLIB/kroC100.tsp")) {
	args.solution = 20749;

    } else if (!strcmp(args.filename, "./TSPLIB/kroD100.tsp")) {
	args.solution = 21294;

    } else if (!strcmp(args.filename, "./TSPLIB/kroA150.tsp")) {
	args.solution = 26524;

    } else if (!strcmp(args.filename, "./TSPLIB/kroA200.tsp")) {
	args.solution = 29368;

    } else if (!strcmp(args.filename, "./TSPLIB/ch150.tsp")) {
	args.solution = 6528;

    } else if (!strcmp(args.filename, "./TSPLIB/rat195.tsp")) {
	args.solution = 2323;

    } else if (!strcmp(args.filename, "./TSPLIB/ts225.tsp")) {
	args.solution = 126643;

    } else if (!strcmp(args.filename, "./TSPLIB/pr226.tsp")) {
	args.solution = 80369;

    } else if (!strcmp(args.filename, "./TSPLIB/pr264.tsp")) {
	args.solution = 49135;

    } else if (!strcmp(args.filename, "./TSPLIB/pr299.tsp")) {
	args.solution = 48191;

    } else if (!strcmp(args.filename, "./TSPLIB/a280.tsp")) {
	args.solution = 2579;

    } else if (!strcmp(args.filename, "./TSPLIB/att532.tsp")) {
	args.solution = 27686;

    } else if (!strcmp(args.filename, "./TSPLIB/rat783.tsp")) {
	args.solution = 8806;

    } else if (!strcmp(args.filename, "./TSPLIB/pr1002.tsp")) {
	args.solution = 259045;

    } else if (!strcmp(args.filename, "./TSPLIB/vm1084.tsp")) {
	args.solution = 239297;

    } else if (!strcmp(args.filename, "./TSPLIB/pr2392.tsp")) {
	args.solution = 378032;

    } else if (!strcmp(args.filename, "./TSPLIB/fl3795.tsp")) {
	args.solution = 28772;

    } else if (!strcmp(args.filename, "./TSPLIB/pcb3038.tsp")) {
	args.solution = 137694;

    } else if (!strcmp(args.filename, "./TSPLIB/fnl4461.tsp")) {
	args.solution = 182566;

    } else if (!strcmp(args.filename, "./TSPLIB/rl5934.tsp")) {
	args.solution = 556045;

    } else if (!strcmp(args.filename, "./TSPLIB/pla7397.tsp")) {
	args.solution = 23260728;

    } else if (!strcmp(args.filename, "./TSPLIB/usa13509.tsp")) {
	args.solution = 19982859;

    } else if (!strcmp(args.filename, "./TSPLIB/d15112.tsp")) {
	args.solution = 1573084;

    } else if (!strcmp(args.filename, "./TSPLIB/d18512.tsp")) {
	args.solution = 645238;

    } else if (!strcmp(args.filename, "./TSPLIB/sw24978.tsp")) {
	args.solution = 855597;

    } else if (!strcmp(args.filename, "./TSPLIB/pla33810.tsp")) {
	args.solution = 66048945;

    } else if (!strcmp(args.filename, "./TSPLIB/pla85900.tsp")) {
	args.solution = 142382641;

    } else if (!strcmp(args.filename, "./TSPLIB/mona-lisa100K.tsp")) {
	args.solution = 5757080;

    } else {
	args.solution = INT_MAX;
    }

    // Summary
    trace("Done.\n");
    trace("----------\n");
    trace("Input filename = %s\n", args.filename);
    trace("Output filename = %s\n", args.outfilename);
    trace("Length of the best known solution = %ld\n", args.solution);
    trace("Pthreads = %d\n", args.pthreads);
    trace("Max number of CPU cores to be used = %d\n", args.maxCoresUsed);
    trace("Sched_setaffinity = %d\n", args.setAffinity);
    trace("Inter-thread communication = %d\n", args.comm);
    trace("Backtracking: %s, StallIterations: %d, BacktrackingLimit: %d,\n",
	  args.backtracking, args.stallIterations, args.backtrackingLimit);
    trace("StallTimePeriod: %d, StallTimeMultiplier: %d\n", args.stallTimePeriod,
	  args.stallTimeMultiplier);
    // trace("GPU Threads = %d\n", args.gpuThreads);
    // trace("GPU Blocks = %d\n", args.gpuBlocks);
    trace("Time limit = %.3f s\n", args.timelimit);
    trace("Error tolerance = %.3f%% \n", args.error * 100.0f);
    trace("Init method = %s\n", args.initMethod);
    trace("----------\n");
    return args;
}
