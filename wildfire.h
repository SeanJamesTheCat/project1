#ifndef WILDFIRE_H
#define WILDFIRE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "display.h"

#define DEF_PRINT -10
#define DEF_SIZE 10
#define DEF_P_CATCH 0.30
#define DEF_DENSITY 0.50
#define DEF_P_BURNING 0.10
#define DEF_P_NEIGHBOR 0.25

#define NUM_OPTIONS 7
#define RAND_SEED 41
#define SLEEP_TIME 750000

#define FAIL EXIT_FAILURE
#define PASS EXIT_SUCCESS

int opterr = 0;

static char options[] = {'H', 'b', 'c', 'd', 'n', 'p', 's'};

static int opt_limits[7][2] = {
	{-10, -10}, //(-H) (-10 means no limit)
	{0, 101},
	{0, 101},
	{0, 101},
	{-1, 101},
	{-1, 10001},
	{4, 41}
};

typedef enum {
	PRINT,
	OVERLAY
} DisplayMode;

typedef enum {
	EMPTY = ' ',
	LIVING = 'Y',
	BURNING = '*',
	BURNED = '.'
} State;

typedef struct cell {
	int r;
	int c;
	State state;
} Cell;

void print_n(char *str, int n) {
	for (int i = 0; i < n; ++i) printf(str);
}

void print_header(int NN) {
	print_n("=", 27);
	printf("\n");
	print_n("=", 8);
	printf(" Wildfire ");
	print_n("=", 9);
	printf("\n");
	print_n("=", 27);
	printf("\n");
}

void print_help(void) {
	printf(
		"usage: wildfire [options]\n"
		"By default, the simulation runs in overlay display mode.\n"
		"the -pN option makes the simulation run in print mode for up to N states.\n"
		"\n"
		"Simulation Configuration Options:\n"
		" -H  # View simulation options and quit.\n"
		" -bN # proportion that a tree is already is already burning. 0 < N < 101.\n"
		" -cN # probability that a tree will catch fire. 0 < N < 101.\n"
		"- dN # density: the proportion of trees in the grid. 0 < N < 101.\n"
		" -nN # proportion of neighbors that influence a tree catching fire. -1 < N < 101.\n"
		" -pN # number of states to print before quitting. -1 < N < ...\n"
		" -sN # simulation grid size. 4 < N < 41.\n"
		"\n"
	);
}






