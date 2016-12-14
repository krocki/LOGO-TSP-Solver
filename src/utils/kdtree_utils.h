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

#define KDTREE_DEFINE_OSTREAM_OPERATORS
#include <headers.h>
#include <kdtree.hpp>

#include <ostream>
#include <vector>
#include <limits>
#include <functional>

using namespace std;

// Kd-tree struct
struct pointStruct {

    inline float    operator[] (int const N) const {
        return d[N];
    } inline bool   operator== (pointStruct const &other) const {
        return this->d[0] == other.d[0] && this->d[1] == other.d[1];
    } inline bool   operator!= (pointStruct const &other) const {
        return this->d[0] != other.d[0] || this->d[1] != other.d[1];
    } friend        ostream & operator<< (ostream & o, pointStruct const &d) {
        return o << "(" << d[0] << "," << d[1] << ")";
    } float         d[2];
    ROUTE_DATA_TYPE id;
};

typedef KDTree::KDTree <2,
						pointStruct,
						std::pointer_to_binary_function <pointStruct, int, double> >
kd_tree;

inline double
return_dup (pointStruct d, int k) {
    return d[k];
}
