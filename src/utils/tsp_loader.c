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

/*
 * The original license, code has been modified to run within Logo TSP Solver
 * 
 * Copyright (c) 2011, Texas State University-San Marcos.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. Redistributions in binary form
 * must reproduce the above copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other materials provided with
 * the distribution. Neither the name of Texas State University-San Marcos nor the
 * names of its contributors may be used to endorse or promote products derived from 
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * Authors: Martin Burtscher and Molly A. O'Neil 
 */
#include <headers.h>

int
readFileCoords(char *filename, city_coords * coords)
{
    register int    i,
                    ch,
                    cnt,
                    cities;
    int             i1;
    float           i2,
                    i3;
    register float *posx,
                   *posy;
    register FILE  *f;
    char            str[512];
    f = fopen(filename, "r+t");

    if (f == NULL) {
	printf("could not open file %s\n", filename);
	exit(-1);
    }
    // this code reads the coordinates properly only if there is at most one line
    // of comments
    // needs an improvement
    ch = getc(f);

    while ((ch != EOF) && (ch != '\n')) {
	ch = getc(f);
    }

    ch = getc(f);

    while ((ch != EOF) && (ch != '\n')) {
	ch = getc(f);
    }

    ch = getc(f);

    while ((ch != EOF) && (ch != '\n')) {
	ch = getc(f);
    }

    ch = getc(f);

    while ((ch != EOF) && (ch != ':')) {
	ch = getc(f);
    }

    int             res = fscanf(f, "%s\n", str);
    cities = atoi(str);

    if (cities == 0) {
	printf("%d cities read, check format\n", cities);
	exit(-1);
    }

    posx = (float *) malloc(sizeof(float) * cities);
    posy = (float *) malloc(sizeof(float) * cities);

    if ((posx == NULL) || (posy == NULL)) {
	printf("out of memory\n");
	exit(-1);
    }

    ch = getc(f);

    while ((ch != EOF) && (ch != '\n')) {
	ch = getc(f);
    }

    res = fscanf(f, "%s\n", str);

    if (strcmp(str, "NODE_COORD_SECTION") != 0) {
	printf("wrong file format\n");
	exit(-1);
    }

    cnt = 0;

    while (fscanf(f, "%d %f %f\n", &i1, &i2, &i3)) {
	posx[cnt] = i2;
	posy[cnt] = i3;
	cnt++;

	if (cnt > cities) {
	    printf("input too long\n");
	    break;
	}

	if (cnt != i1) {
	    printf("input line mismatch: expected %d instead of %d\n", cnt, i1);
	}
    }

    if (cnt != cities) {
	printf("read %d instead of %d cities\n", cnt, cities);
    }

    res = fscanf(f, "%s", str);

    if (strcmp(str, "EOF") != 0) {
	printf("didn't see 'EOF' at end of file\n");
    }

    fclose(f);

    for (i = 0; i < cities; i++) {
	coords[i].x = posx[i];
	coords[i].y = posy[i];
    }

    free(posx);
    free(posy);
    return cities;
}
