#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define nil NULL
#define print printf
#define exits(x) { fprintf(stderr, x); exit(-1); }
#else
#include <u.h>
#include <libc.h>
#endif
#include "vmatrix.h"

enum {
	WHITE = 'w',
	BLACK = 'b',
};

// recursive function to change solitary white neighbors
// of vin[p].  assumes vin[p] is black.  tail-recursion(?)
// returns 0 for zero-forcing sets
int change(Matrix *m, unsigned int p, char *vin) {
	unsigned register q;
	unsigned int wneighbor = 0;
	unsigned int wneighbors = 0;

	if (strchr(vin, WHITE) == nil) {
		print ("is a zero-forcing set");
		return 0;
	}

	// count white neighbors
	for (q = 0; q < m->r; q++) {
		if (q == p)
			continue;

		if ((m->d[p][q] != 0.0)
		  && (vin[q] == WHITE)) {
			wneighbors++;
			wneighbor = q;
		}
	}

	// if one white neighbor, switch it
	if (wneighbors == 1) {
		vin[wneighbor] = BLACK;
		// follow the chain reaction
		return change(m, wneighbor, vin);		
	}

	return 1;
}

void check(Matrix *m, char *vin) {
	unsigned register q;

	print("checking %s... ", vin);

	for (q = 0; vin[q] != '\0'; q++) {
		if (vin[q] == BLACK) {
			if (change(m, q, vin) == 0)
				break;
		}
	}

	print("\n");
}

// recursive function to generate combinations
void pick(Matrix *m, char *vin, unsigned int p, unsigned int l) {
	unsigned register q;
	char *vals;

	for (q = p; vin[q] != '\0'; q++) {
		vals = strdup(vin);

		if (vals[q] == WHITE) {
			vals[q] = BLACK;

			if (l == 1) {
				check(m, vals);

			} else {
				pick(m, vals, q, l-1);

			}
		}
		free(vals);
	}
}

void combos(Matrix *m) {
	unsigned register q;
	char *vals;

	for (q = 1; q < m->r; q++) {
		vals = malloc(m->r + 1);
		vals[m->r] = '\0';
		memset(vals, WHITE, m->r);

		pick(m, vals, 0, q);

		free(vals);
	}
}

int main() {
	Matrix *m = scanmat();

	if (m == nil)
		exits("matrix read error");

	if (m->r != m->c) {
		exits("matrix not square");
	}

	print ("\n");
	combos(m);

	delmat(m);
	return 0;
}
