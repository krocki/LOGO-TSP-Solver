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
 *
 *
 *   The following code uses libkdtree++, a C++ template KD-Tree sorting container.
 *   libkdtree++ is (c) 2004-2007 Martin F. Krafft <libkdtree@pobox.madduck.net>
 *   and Sylvain Bougerel <sylvain.bougerel.devel@gmail.com> distributed under the
 *   terms of the Artistic License 2.0.
 */


#include <kdtree_utils.h>

static kd_tree  tree(std::ptr_fun(return_dup));

// KD tree construction - O(nlogn)
void
constructKDTree(city_coords * coords, ROUTE_DATA_TYPE cities)
{
    std::vector < pointStruct > vector_with_points;

    for (ROUTE_DATA_TYPE i = 0; i < cities; i++) {
	// new pointStruct
	pointStruct     point = { {coords[i].x, coords[i].y}
	};
	point.id = i;
	// check if pointStruct with same coordinate already in vector/tree. If not: 
	// insert in vector and tree
	kd_tree::iterator pItr =
	    tree.find_nearest(point, std::numeric_limits < double >::max()).first;

	if (*pItr != point) {
	    tree.insert(point);
	    vector_with_points.push_back(point);
	}
    }

    tree.optimise();
}

// TODO: clean this up a little bit, possibly encapsulate some operations in
// functions or/and macros
vector < ROUTE_DATA_TYPE > multipleFragment_KD(ROUTE_DATA_TYPE cities,
					       city_coords * coords)
{
    ROUTE_DATA_TYPE *degree;
    ROUTE_DATA_TYPE *NNLink;
    ROUTE_DATA_TYPE *tail;
    long           *pred;
    long           *suc;
    short          *init;
    degree = (ROUTE_DATA_TYPE *) malloc(sizeof(ROUTE_DATA_TYPE) * MAX_CITIES);
    NNLink = (ROUTE_DATA_TYPE *) malloc(sizeof(ROUTE_DATA_TYPE) * MAX_CITIES);
    tail = (ROUTE_DATA_TYPE *) malloc(sizeof(ROUTE_DATA_TYPE) * MAX_CITIES);
    init = (short *) malloc(sizeof(short) * MAX_CITIES);
    pred = (long *) malloc(sizeof(long) * MAX_CITIES);
    suc = (long *) malloc(sizeof(long) * MAX_CITIES);
    priority_queue < fragment, vector < fragment >, CompareFragments > pq;

    for (ROUTE_DATA_TYPE i = 0; i < cities; i++) {
	degree[i] = 0;
	init[i] = -1;
	suc[i] = -1;
	pred[i] = -1;
	tail[i] = i;
	NNLink[i] = getNN_KD(i, coords);
	fragment        f;
	f.distance = calculateDistance2D(i, NNLink[i], coords);
	f.id = i;
	pq.push(f);
    }

    // copy the tree
    kd_tree         tree_copy(tree);
    pointStruct     erased_tail;
    // loop N - 1 times

    for (ROUTE_DATA_TYPE i = 0; i < cities - 1; i++) {
	ROUTE_DATA_TYPE x;
	ROUTE_DATA_TYPE y;

	// loop
	while (1) {
	    // pq -> getTop...
	    fragment        f = pq.top();
	    x = f.id;

	    // if Degree[x] = 2 remove top, continue...
	    if (degree[x] == 2) {
		pq.pop();
		continue;
	    }

	    y = NNLink[x];

	    if (degree[y] < 2 && y != tail[x]) {
		break;		// y is valid neighbor to x

	    } else if (init[x] != -1) {	// hide own tail
		pointStruct     point_to_be_erased =
		    { {coords[tail[x]].x, coords[tail[x]].y}
		};
		point_to_be_erased.id = tail[x];
		kd_tree::iterator pItr =
		    tree_copy.find_nearest(point_to_be_erased,
					   std::numeric_limits <
					   double >::max()).first;
		tree_copy.erase_exact(((struct pointStruct) *(pItr)));
		erased_tail = point_to_be_erased;
	    }
	    // get NN to x
	    pointStruct     point = { {coords[x].x, coords[x].y} };
	    point.id = x;
	    kd_tree::iterator pItr =
		tree_copy.find_nearest(point,
				       std::numeric_limits < double >::max()).first;
	    tree_copy.erase_exact(((struct pointStruct) *(pItr)));
	    pItr =
		tree_copy.find_nearest(point,
				       std::numeric_limits < double >::max()).first;
	    tree_copy.insert(point);
	    NNLink[x] = ((struct pointStruct) *(pItr)).id;

	    if (init[x] != -1) {	// put tail back
		tree_copy.insert(erased_tail);
	    }

	    pq.pop();
	    f.distance = calculateDistance2D(x, NNLink[x], coords);
	    f.id = x;
	    pq.push(f);
	}			// loop end

	degree[x]++;
	degree[y]++;

	// add edges (x, y)

	if (init[x] == -1) {
	    pred[x] = y;

	} else {
	    suc[x] = y;
	}

	if (init[y] == -1) {
	    pred[y] = x;

	} else {
	    suc[y] = x;
	}

	// exclude the edges with degrees == 2

	if (degree[x] == 2) {
	    pointStruct     point_to_be_erased = { {coords[x].x, coords[x].y}
	    };
	    point_to_be_erased.id = x;
	    kd_tree::iterator pItr =
		tree_copy.find_nearest(point_to_be_erased,
				       std::numeric_limits < double >::max()).first;
	    tree_copy.erase_exact(((struct pointStruct) *(pItr)));
	}

	if (degree[y] == 2) {
	    pointStruct     point_to_be_erased = { {coords[y].x, coords[y].y}
	    };
	    point_to_be_erased.id = y;
	    kd_tree::iterator pItr =
		tree_copy.find_nearest(point_to_be_erased,
				       std::numeric_limits < double >::max()).first;
	    tree_copy.erase_exact(((struct pointStruct) *(pItr)));
	}
	// update tails array
	ROUTE_DATA_TYPE temp = tail[x];
	tail[temp] = tail[y];
	tail[tail[y]] = temp;
	init[x] = 0;
	init[y] = 0;
    }				// loop N - 1 times end

    // assemble route_a - go forward and route_b - go backward
    // and combine them
    vector < ROUTE_DATA_TYPE > route_a;
    vector < ROUTE_DATA_TYPE > route_b;
    route_a.push_back(0);
    long            y = 0;
    long            x;
    x = pred[y];

    do {
	if (suc[y] == x) {
	    suc[y] = pred[y];
	    pred[y] = x;
	}

	x = y;

	if (y > 0) {
	    route_a.push_back((ROUTE_DATA_TYPE) y);
	}
    } while ((y = suc[x]) != -1);

    y = 0;
    x = suc[y];

    do {
	if (pred[y] == x) {
	    pred[y] = suc[y];
	    suc[y] = x;
	}

	x = y;

	if (y > 0) {
	    route_b.push_back((ROUTE_DATA_TYPE) y);
	}
    } while ((y = pred[x]) != -1);

    reverse(route_b.begin(), route_b.end());
    route_a.insert(route_a.begin(), route_b.begin(), route_b.end());
    free(degree);
    free(tail);
    free(NNLink);
    free(init);
    free(pred);
    free(suc);
    return route_a;
}
// get the list of the howMany nearest elements to the point (exclude the point
// itself) - O(logn) - sorted
vector < ROUTE_DATA_TYPE > getNN_KD_list(ROUTE_DATA_TYPE index, city_coords * coords,
					 ROUTE_DATA_TYPE howMany)
{
    // define the point
    pointStruct     point = { {coords[index].x, coords[index].y}
    };
    point.id = index;
    vector < ROUTE_DATA_TYPE > returnIds;
    std::vector < pointStruct > vector_with_points;
    vector_with_points.push_back(point);
    // remove the point before the search
    tree.erase_exact(point);

    while (howMany-- > 0 && tree.size() > 0) {
	// get NN of that point
	kd_tree::iterator pItr =
	    tree.find_nearest(point, std::numeric_limits < double >::max()).first;
	// cout << "KDlist : data nn: " << ((struct pointStruct)*(pItr)).id << endl;
	returnIds.push_back(((struct pointStruct) *(pItr)).id);
	// cout << "KDlist: data nn: " << *pItr << endl;
	// remove the next point before the search
	vector_with_points.push_back(((struct pointStruct) *(pItr)));
	tree.erase_exact(((struct pointStruct) *(pItr)));
    }

    // reinsert the points
    while (vector_with_points.size() > 0) {
	tree.insert(vector_with_points.back());
	vector_with_points.pop_back();
    }

    return returnIds;
}

vector < ROUTE_DATA_TYPE > initRoute_KD_NN(ROUTE_DATA_TYPE size,
					   city_coords * coords)
{
    vector < ROUTE_DATA_TYPE > route;
    kd_tree         tree_copy(tree);
    // first
    ROUTE_DATA_TYPE current = 0;
    pointStruct     point = { {coords[current].x, coords[current].y}
    };
    point.id = current;

    while (tree_copy.size() > 0) {
	kd_tree::iterator pItr =
	    tree_copy.find_nearest(point,
				   std::numeric_limits < double >::max()).first;
	route.push_back(((struct pointStruct) *(pItr)).id);
	tree_copy.erase_exact(((struct pointStruct) *(pItr)));
	point.d[0] = coords[route.back()].x;
	point.d[1] = coords[route.back()].y;
    }

    return route;
}


// get the second nearest element to the point (exclude the point itself)
ROUTE_DATA_TYPE
getNN_KD(ROUTE_DATA_TYPE index, city_coords * coords)
{
    // define the point
    pointStruct     point = { {coords[index].x, coords[index].y}
    };
    point.id = index;
    // remove the point before the search
    tree.erase_exact(point);
    // get NN of that point
    kd_tree::iterator pItr =
	tree.find_nearest(point, std::numeric_limits < double >::max()).first;
    // reinsert the point
    tree.insert(point);
    return ((struct pointStruct) *(pItr)).id;
}


void
removePointKD(city_coords * coords, ROUTE_DATA_TYPE index)
{
    pointStruct     point = { {coords[index].x, coords[index].y} };
    point.id = index;
    tree.erase_exact(point);
}

void
removePointsKDTree(city_coords * coords, vector < ROUTE_DATA_TYPE > &points)
{
    std::vector < pointStruct > vector_with_points;

    for (ROUTE_DATA_TYPE i = 0; i < points.size(); i++) {
	// new pointStruct
	pointStruct     point = { {coords[i].x, coords[i].y}
	};
	point.id = i;
	vector_with_points.push_back(point);
    }

    size_t          elements;

    while (vector_with_points.size() > 0) {	// delete all pointStructs from tree 
						// which are in the vector
	elements = vector_with_points.size();
	pointStruct     element_to_erase = vector_with_points.back();
	vector_with_points.pop_back();
	tree.erase_exact(element_to_erase);
	// some debug code
	// now check that it cannot find the element UNLESS there is another one
	// with the identical location in the list...
	/*
	 * if
	 * (find(vector_with_points.begin(),vector_with_points.end(),element_to_erase) 
	 * == vector_with_points.end()) { kd_tree::iterator not_there =
	 * tree.find(element_to_erase); if (not_there != tree.end()) { cout <<
	 * "SHOULD NOT HAVE FOUND THIS: " << *not_there << endl; assert(0); } else {
	 * cout << " find() double-check passed." << endl; } } 
	 */
    }
}
