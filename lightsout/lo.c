#if defined(__unix__) || defined(unix)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#define nil NULL
#define print printf
#define sysfatal(x) { fprintf(stderr, x); fprintf(stderr, "\n"); exit(-1); }
#else
#include <u.h>
#include <libc.h>
#endif
// matrix helper functions
#include "vmatrix.h"

// these definitions refer to character values in strings defining
// vertex colorings
enum {
	WHITE = 'w',
	BLACK = 'b',
};

// global "found" flag to leave combos()
char found = 0;
// to leave check()
char foundone;

// recursive function to change solitary white neighbors
// of vin[p].  assumes vin[p] is black.
// returns 1 for zero-forcing sets
int change(Matrix *m, unsigned int p, char *vin) {
	unsigned register q;
	unsigned int wneighbor = 0;
	unsigned int wneighbors = 0;
	unsigned int r = 0;

	// found one in this state space
	if (foundone)
		return 0;

	// if there are no white vertices left...
	if (strchr(vin, WHITE) == nil) {
		// we started with a zero-forcing set
		return 1;
	}

	// count white neighbors
	for (q = 0; q < m->r; q++) {
		if (q == p)
			continue;

		if ((m->d[p][q] != 0.0)
		  && (vin[q] == WHITE)) {
			wneighbors++;

			// Nathan's suggestion; if we have two white neighbors,
			// no need to keep checking
			if (wneighbors > 1)
				break;

			wneighbor = q;
		}
	}

	// if one white neighbor, switch it
	if (wneighbors == 1) {
		vin[wneighbor] = BLACK;

		// check them all again if we made a change
		for (q = 0; q < m->r; q++)
			if (vin[q] == BLACK)
				if (change(m, q, vin) == 1)
					return 1;
	}

	return 0;
}

// check against m if vin is a zero-forcing set
void check(Matrix *m, char *vin) {
	unsigned register q;
	char *vals;

	// we have not found one while checking this combo
	foundone = 0;

	// this is where the parallelization occurs.
	// this for loop over each vertex gets split in OMP_NUM_THREADS
#pragma omp parallel for private(vals)
	for (q = 0; q < m->r; q++) {
		// we don't break out of this loop, branching like that is
		// not allowed.  that's what foundone var is for.  otherwise
		// checks each black vertex against vin vertex color values
		if (!foundone && vin[q] == BLACK) {
			vals = strdup(vin);
			if (change(m, q, vals) == 1) {
				print ("%s\n", vin);
// for correct piping to other programs
#if defined(__unix__) || defined(unix)
				fflush (stdout);
#endif
				found = 1;
				foundone = 1;
			}
			free(vals);
		}
	}
}

// recursive function to pick l vertices in vin and turn them black
void pick(Matrix *m, char *vin, unsigned int p, unsigned int l) {
	unsigned register q;
	char *vals;

	for (q = p; q < m->r; q++) {
		if (vin[q] == WHITE) {
			// copy vin to vals
			vals = strdup(vin);
			vals[q] = BLACK;

			if (l == 1) {
				// leafs of state space check m against current
				// vertex color values.
				check(m, vals);

			} else {
				pick(m, vals, q, l-1);

			}

			free(vals);
		}
	}
}

// generate all permutations of 1 through N-1 black vertices
void combos(Matrix *m) {
	unsigned register q;
	char *vals;

	// stop if we found least number of vertices needed
	// by observing variable `found'
	for (q = 1; q < m->r, !found; q++) {
		vals = malloc(m->r + 1);
		vals[m->r] = '\0';
		memset(vals, WHITE, m->r);

		// next step is to pick q vertices to turn black this 
		// function calls check(adjacency matrix, vertex colors) too
		pick(m, vals, 0, q);

		free(vals);
	}
}

int main() {
	// scan a matrix in using format of vmatrix.o
	Matrix *m = scanmat();

	if (m == nil)
		sysfatal("matrix read error");

	if (m->r != m->c) {
		sysfatal("matrix not square");
	}

<<<<<<< HEAD
	// main subroutine
=======
	// enter the matrix.  (ha)
>>>>>>> 550b9b4a4367475320574a64dd5fc963726719a9
	combos(m);

	delmat(m);
	return 0;
}
