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

/*
 * Multiple Fragment Heuristic
 *
 * @ARTICLE{ jouis bentley:fast,
 * AUTHOR = "Jon Jouis Bentley",
 * TITLE = "Fast Algorithms for Geometric Traveling Salesman Problems",
 * JOURNAL = "INFORMS Journal on Computing",
 * PAGES = {387-411},
 * YEAR = {1992},
 * }
 *
 */

/*
 * @inproceedings{Bentley:1990:ETS:320176.320186,
 * author = {Bentley, Jon Louis},
 * title = {Experiments on traveling salesman heuristics},
 * booktitle = {Proceedings of the first annual ACM-SIAM symposium on Discrete algorithms},
 * series = {SODA '90},
 * year = {1990},
 * isbn = {0-89871-251-3},
 * location = {San Francisco, California, United States},
 * pages = {91--99},
 * numpages = {9},
 * url = {http://dl.acm.org/citation.cfm?id=320176.320186},
 * acmid = {320186},
 * publisher = {Society for Industrial and Applied Mathematics},
 * address = {Philadelphia, PA, USA},
 * }
 *
 */

vector < ROUTE_DATA_TYPE > multipleFragment(ROUTE_DATA_TYPE cities,
					    city_coords * coords)
{
    // see kdtree_utils.c
    return multipleFragment_KD(cities, coords);
}

// just a simple O(n) algorithm - get the closest point
ROUTE_DATA_TYPE
getNNSimple(ROUTE_DATA_TYPE index, city_coords * coords, ROUTE_DATA_TYPE cities)
{
    float           min = 99999;
    ROUTE_DATA_TYPE j = 0;

    for (ROUTE_DATA_TYPE i = 0; i < cities; i++) {
	float           temp = calculateDistance2D(index, i, coords);

	if (i != index)
	    if (temp < min) {
		min = temp;
		j = i;
	    }
    }

    return j;
}

// fast NN algorithm based on kd-tree search - O(nlogn)
vector < ROUTE_DATA_TYPE > routeInitNN_KD(ROUTE_DATA_TYPE size, city_coords * coords)
{
    return initRoute_KD_NN(size, coords);
}

// simple naive NN tour construction algorithm - O(n^2)
vector < ROUTE_DATA_TYPE > routeInitNN(ROUTE_DATA_TYPE size, city_coords * coords)
{
    vector < ROUTE_DATA_TYPE > temp, route;
    // resize the vector
    temp.resize(size - 1);
    // fill the vector with numbers
    iota(temp.begin(), temp.end(), START_NUMBERING_FROM + 1);
    ROUTE_DATA_TYPE index = 0;
    ROUTE_DATA_TYPE best;
    unsigned long   cost;
    // insert first point
    route.push_back(0);

    while (temp.size() > 0) {
	best = 0;
	cost = calculateDistance2D(route[index], temp[best], coords);

	for (ROUTE_DATA_TYPE i = 0; i < temp.size(); i++) {
	    if (calculateDistance2D(route[index], temp[i], coords) < cost) {
		cost = calculateDistance2D(route[index], temp[i], coords);
		best = i;
	    }
	}

	route.push_back(temp[best]);
	temp.erase(temp.begin() + best, temp.begin() + best + 1);
	index++;
    }

    return route;
}

void
routeInit(vector < ROUTE_DATA_TYPE > &route, int method, ROUTE_DATA_TYPE size,
	  city_coords * coords)
{
    // methods
    // ROUTE_INIT_SIMPLE 0
    // ROUTE_INIT_NN 1
    // ROUTE_INIT_SHUFFLE 2
    // ROUTE_INIT_MF 3
    trace("Constructing initial route...\n");

    if (method == ROUTE_INIT_SIMPLE || method == ROUTE_INIT_SHUFFLE) {
	// resize the vector
	route.resize(size);
	// fill the vector with numbers
	iota(route.begin(), route.end(), START_NUMBERING_FROM);

	if (method == ROUTE_INIT_SHUFFLE) {
	    random_shuffle(route.begin(), route.end());
	}

    } else if (method == ROUTE_INIT_NN) {
	// nearest neighbor algorithm
	// routeInitNN_KD or routeInitNN
	route = routeInitNN_KD(size, coords);

    } else if (method == ROUTE_INIT_MF) {
	// multiple fragment heuristic
	route = multipleFragment(size, coords);
    }

    trace("Done (Length = %ld).\n", routeLength(route, coords));
}

// check the validity in O(nlogn) time
void
checkTour(vector < ROUTE_DATA_TYPE > route)
{
    // checks if the route is valid
    trace("Checking the tour...\n");
    // sort the ids
    sort(route.begin(), route.end());

    // check the first and the last one

    if (route.size() < 3) {
	trace("Tour is invalid! (too short)\n");
	exit(-1);

    } else if (route[0] != 0 || route.back() != (route.size() - 1)) {
	trace("Tour is invalid! (missing elements)\n");
	exit(-1);

    } else {
	// check if there are any repeating elements
	for (ROUTE_DATA_TYPE i = 0; i < route.size() - 1; i++) {
	    if (route[i] == route[i + 1]) {
		trace("Tour is invalid! (repeating %d)\n", route[i]);
		exit(-1);
	    }
	}
    }

    // ok
    trace("Tour is valid.\n");
}
