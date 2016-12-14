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

static struct timeval start_time;

double
getTimeDiff(struct timeval *s, struct timeval *e)
{
    // computes time difference between s and e
    struct timeval  diff_tv;
    diff_tv.tv_usec = e->tv_usec - s->tv_usec;
    diff_tv.tv_sec = e->tv_sec - s->tv_sec;

    if (s->tv_usec > e->tv_usec) {
	diff_tv.tv_usec += 1000000;
	diff_tv.tv_sec--;
    }

    return (double) diff_tv.tv_sec + ((double) diff_tv.tv_usec / 1000000.0);
}

void
initStartTime(void)
{
    // set the program start time
    gettimeofday(&start_time, NULL);
    trace("Timer reset\n");
}

double
getTotalTime(void)
{
    struct timeval  s;
    gettimeofday(&s, NULL);
    double          time = getTimeDiff(&start_time, &s);
    return time;
}

void
printCurrentLocalTime(void)
{
    // print current global and local time
    struct timeval  s;
    gettimeofday(&s, NULL);
    double          time = getTimeDiff(&start_time, &s);
    time_t          tt = (time_t) s.tv_sec;
    struct tm      *sec;
    sec = localtime(&tt);
    printf("[%02d/%02d/%02d %02d:%02d:%02d.%06ld] %.6f | ", sec->tm_year % 100,
	   sec->tm_mon + 1, sec->tm_mday, sec->tm_hour, sec->tm_min, sec->tm_sec,
	   (long) s.tv_usec, time);
}

void
showCoordinates(city_coords * coords, ROUTE_DATA_TYPE rangeStart,
		ROUTE_DATA_TYPE rangeEnd)
{
    for (ROUTE_DATA_TYPE i = rangeStart; i < rangeEnd; i++) {
	trace("coords[%d].x = %f, coords[%d].y = %f\n", i, coords[i].x, i,
	      coords[i].y);
    }
}

void
logToFile(char *filename, char *string)
{
    register FILE  *f;
    char            fn[64];
    sprintf(fn, "%s.txt", filename);
    f = fopen(fn, "a+");

    if (f == NULL) {
	printf("could not open file %s\n", filename);
	exit(-1);
    }

    struct timeval  s;

    gettimeofday(&s, NULL);

    double          time = getTimeDiff(&start_time, &s);

    // time_t tt = (time_t)s.tv_sec;
    // struct tm *sec;
    // sec = localtime(&tt);
    // fprintf(f,"[%2d.%2d %2d:%2d:%2d.%06ld] %.6f | %s", sec->tm_mon, sec->tm_mday, 
    // sec->tm_hour, sec->tm_min, sec->tm_sec, s.tv_usec, time, string);
    fprintf(f, "%.6f, %s", time, string);

    fclose(f);
}

void
routeInitFromFile(vector < ROUTE_DATA_TYPE > &route, ROUTE_DATA_TYPE size,
		  char *filename)
{
    register FILE  *f;
    ROUTE_DATA_TYPE id = 0;
    f = fopen(filename, "r+t");

    if (f == NULL) {
	printf("could not open file %s\n", filename);
	exit(-1);
    }

    for (ROUTE_DATA_TYPE i = 0; i < size; i++) {
	if (fscanf(f, "%u\n", &id)) {
	    route.push_back(id - 1);
	}
    }

    fclose(f);
}

void
printRoute(vector < ROUTE_DATA_TYPE > &route, city_coords * coords)
{
    printRoute(route, 0, route.size(), coords);
}

void
printRoute(vector < ROUTE_DATA_TYPE > &route, ROUTE_DATA_TYPE rangeStart,
	   ROUTE_DATA_TYPE rangeEnd, city_coords * coords)
{
    trace("Route (%d-%d):\n", rangeStart + 1, rangeEnd);

    for (ROUTE_DATA_TYPE i = rangeStart; i < rangeEnd; i++) {
	printf("%d, length (%d->%d): %ld\n", route[i] + 1, rangeStart, rangeEnd,
	       routeLength(route, coords, 0, i));
    }
}

void
saveRouteToAFile(vector < ROUTE_DATA_TYPE > &route, char *filename,
		 unsigned long length)
{
    saveRouteToAFile(route, 0, route.size(), filename, length);
}

void
saveRouteToAFile(vector < ROUTE_DATA_TYPE > &route, ROUTE_DATA_TYPE rangeStart,
		 ROUTE_DATA_TYPE rangeEnd, char *filename, unsigned long length)
{
    char            fn[64];
    sprintf(fn, "%s_%ld.tour", filename, length);
    FILE           *file;
    file = fopen(fn, "w");

    for (ROUTE_DATA_TYPE i = rangeStart; i < rangeEnd; i++) {
	fprintf(file, "%d\n", route[i] + 1);
    }

    fclose(file);
}

void
saveRouteToAFile(vector < ROUTE_DATA_TYPE > &route, char *filename)
{
    saveRouteToAFile(route, 0, route.size(), filename);
}

void
saveRouteToAFile(vector < ROUTE_DATA_TYPE > &route, ROUTE_DATA_TYPE rangeStart,
		 ROUTE_DATA_TYPE rangeEnd, char *filename)
{
    char            fn[64];
    sprintf(fn, "%s.tour", filename);
    FILE           *file;
    file = fopen(fn, "w");

    for (ROUTE_DATA_TYPE i = rangeStart; i < rangeEnd; i++) {
	fprintf(file, "%d\n", route[i] + 1);
    }

    fclose(file);
}

// taken from
// http://stackoverflow.com/questions/83439/remove-spaces-from-stdstring-in-c
string
delUnnecessary(string & str)
{
    int             size = str.length();

    for (int j = 0; j <= size; j++) {
	for (int i = 0; i <= j; i++) {
	    if (str[i] == ' ' && str[i + 1] == ' ') {
		str.erase(str.begin() + i);

	    } else if (str[0] == ' ') {
		str.erase(str.begin());

	    } else if (str[i] == '\0' && str[i - 1] == ' ') {
		str.erase(str.end() - 1);
	    }
	}
    }

    return str;
}

string
getFirstWord(string & str)
{
    return str.substr(0, str.find(' '));
}

string
getCPUInfo(void)
{
    string          line;
#ifdef __linux__
    ifstream        finfo("/proc/cpuinfo");

    while (getline(finfo, line)) {
	stringstream    str(line);
	string          itype;
	string          info;

	if (getline(str, itype, ':') && getline(str, info)
	    && itype.substr(0, 10) == "model name") {
	    return delUnnecessary(info);
	}
    }

#endif
#ifdef __APPLE__
    char            buf[100];
    size_t          buflen = 100;
    sysctlbyname("machdep.cpu.brand_string", &buf, &buflen, NULL, 0);
    std::string info = string(buf);
    return delUnnecessary(info);
#endif
    return string("cpu name");
}

void
initRand(void)
{
    struct timeval  t;
    gettimeofday(&t, NULL);
    srand((t.tv_sec * 1000) + (t.tv_usec / 1000));
}

void
printRankNode(void)
{
    // USED IN MPI ENVIRONMENT
    // printf("%3d | %10s | ", rank, processor_name);
}

void
printLicense(void)
{
    printf("\nLogo TSP Solver ver. 0.62; Copyright (C) 2013 Kamil Rocki.\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
    printf
	("This is free software, and you are welcome to redistribute it under certain\n");
    printf
	("conditions. Please see <http://www.gnu.org/copyleft/gpl.html> for details.\n\n");
}

void
printUsage(void)
{
    printf("\tBasic Usage:\n");
    printf("\n\t./logo --in=FILENAME --initMethod=METHOD --mode=MODE\n");
    printf("\n\tFILENAME - input file (TSPLIB format)\n");
    printf
	("\tMETHOD - init tour construction method {random, nn, mf}, default --initMethod=%s\n",
	 STR(DEFAULT_INIT));
    printf("\tMODE - {cuda, cl, cpu, benchmark}, default --mode=cpu\n\n");
    printf("\t\tcuda - Run on a CUDA device(s)\n");
    printf("\t\tcl - Run on a OpenCL device(s)\n");
    printf("\t\tcpu - Run 'normal' CPU code\n");
    printf
	("\t\tcpumt - Multithreaded CPU, can be combined with --maxCoresCPU=N option\n");
    printf
	("\t\tcpupar - Parallel CPU Local Optimization, can be combined with --maxCoresCPU=N option\n");
    printf("\t\tall - Run on all types of devices\n");
    printf
	("\t\t--benchmark=N - a single optimization step run on all devices measuring time, repeat N times\n");
    printf
	("\n\ti.e. ./logo --in=\"./TSPLIB/vm1084.tsp\" --initMethod=\"mf\" --mode=\"cuda\" --benchmark\n");
    printf("\n\tAt least the input file has to be specified\n");
    printf("\n\tOther options:\n\n");
    printf("\n");
    printf("\t--device=N, specify device ID\n");
    printf("\t--autoDevice, choose device(s) automatically\n");
    printf
	("\t--maxDevices=N, set the maximum number of devices (cuda, cl, ...) used\n");
    printf("\n");
    printf("\t--err=ERROR_TOLERANCE, default --err=%s\n",
	   STR(DEFAULT_ERROR_TOLERANCE));
    printf("\n");
    printf("\t--out=OUTPUT_FILE_PREFIX, default --out=%s\n", STR(DEFAULT_OUT));
    printf
	("\t--initRouteFromFile={0,1}, init from a file in TSPLIB tour format, default --initRouteFromFile=0\n");
    printf("\t--initRouteFile=FILE.tour, i.e. --initRouteFile=\"savedTour.tour\"\n");
    printf("\n");
    printf("\t--blocks=GPU_BLOCKS, default --blocks=%s\n", STR(DEFAULT_GPU_BLOCKS));
    printf
	("\t\tCurrently optimized for Kepler GPUs, may not work properly if changed\n");
    printf("\t--threads=GPU_THREADS_PER_BLOCK, default --threads=%s\n",
	   STR(DEFAULT_GPU_THREADS));
    printf
	("\t--showLOInfo={0,1}, default --showLOInfo=%s, enable/disable Local Optimization messages\n",
	 STR(DEFAULT_SHOW_LO_INFO));
    printf
	("\t--trackSolution={0,1}, default --trackSolution=%s, enable/disable writing local minima to file\n",
	 STR(DEFAULT_TRACK_SOLUTION));
    printf("\n");
    printf
	("\t--pthreads=N, default --pthreads=%s, N - multiple CPU threads for multi-start algorithm\n",
	 STR(DEFAULT_PTHREADS));
    printf
	("\t\t i.e ./logo --in=\"./TSPLIB/vm1084.tsp\" --initMethod=\"mf\" --mode=\"cpu\" --pthreads=6\n");
    printf
	("\t--setAffinity={0,1}, default --setAffinity=%s, set CPU affinity (assign thread to core)\n",
	 STR(DEFAULT_AFFINITY));
    printf
	("\t--maxCoresCPU=N, default --maxCoresCPU=%s, limit the number of CPU cores used\n",
	 STR(DEFAULT_MAX_CORES));
    // printf("\n");
    // printf("\t--backtracking={NO,iter,time}, default --backtracking=%s, go back a 
    // few moves if stuck\n", STR(DEFAULT_BACKTRACKING));
    // printf("\t--backtrackingLimit=N, default --backtrackingLimit=%s, limit the
    // number of back moves rand() %% (total/N)\n",
    // STR(DEFAULT_BACKTRACKING_LIMIT));
    // printf("\t\tN - Disabled, iter - iteration no based, time - time interval
    // based\n");
    // printf("\t--stallIterations=N, default --stallIterations=%s, move backwards
    // every N iteration with no improvement\n", STR(DEFAULT_STALL_ITERS));
    // printf("\t\t(valid only with --backtracking=iter)\n");
    // printf("\t--timePeriod=M, default --timePeriod=%s, consider last M time
    // intervals (to find a local minimum)\n", STR(DEFAULT_STALL_TIME_PERIOD));
    // printf("\t--timeMultiplier=N, default --timeMultiplier=%s, move backwards if
    // N * avg time from last M time intervals\n",
    // STR(DEFAULT_STALL_TIME_MULTIPLIER));
    // printf("\t\t(valid only with --backtracking=time)\n");
    // printf("\t--maxKicks=N, limit maximum random kicks used to perturb the
    // route\n");
    printf
	("\t--comm={0,1}, default --comm=%s, enable/disable inter-thread (within one node) communication\n",
	 STR(DEFAULT_PTHREADS_COMM));
    // printf("\t--commPart=N, default --commPart=%s, specify the part of the
    // threads which communicate\n", STR(DEFAULT_PTHREADS_COMM_PART));
    // printf("\t\ti.e. --commPart=0.5, --commPart=0.25\n");
    // printf("\t\tThe other threads DO NOT read the global solution, but they DO
    // update it\n");
    // printf("\t\tExample: ./logo --in=\"./TSPLIB/pr299.tsp\" --initMethod=\"mf\"
    // --setAffinity=1 --mode=\"cpu\" --pthreads=6 --comm=1 --commPart=0.5
    // --backtracking=time\n");
    printf("\n");
}
