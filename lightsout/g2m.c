#if defined(__unix__) || defined(unix)
#include <stdlib.h>
#define nil NULL
#define print printf
#else
#include <u.h>
#include <libc.h>
#endif
#include <stdio.h>
#include "vmatrix.h"

int main() {
	unsigned int vertices, r, c;

	// read number of vertices
	if (scanf("%u", &vertices) != 1){
		print("invalid vertex count\n");
		return -1;
	}

	Matrix *m = newmat(vertices, vertices);

	// read edges
	while ((scanf("%u", &r) == 1) 
	    && (scanf("%u", &c) == 1)){
		m->d[r][c] = m->d[c][r] = 1;
	}

	printmat(m);
	delmat(m);

	return 0;
}
