#define _DEFAULT_SOURCE

#include "wildfire.h"

static int 
	size = DEF_SIZE, //default: 10
	num_updates = DEF_PRINT,
	cycle = 0,
	changes = 0,
	cum_changes = 0;

static double
	pCatch = DEF_P_CATCH,
	density = DEF_DENSITY,
	pBurning = DEF_P_BURNING,
	pNeighbor = DEF_P_NEIGHBOR;

static DisplayMode mode = OVERLAY;

bool roll_success(double prob) {
	srandom(RAND_SEED);
	double roll = (double) random() / RAND_MAX;
	return roll <= prob;
}

void print_stats(void) {
	if (mode == OVERLAY) set_cur_pos(size + 1, 0);
	printf(
		"size %d, pCatch %.2f, density %.2f, pBurning %.2f, pNeighbor %.2f\n"
		"cycle %d, current changes %d, cumulative changes %d\n",
		size, pCatch, density, pBurning, pNeighbor,
		cycle, changes, cum_changes
	);
}

void print_grid(Cell grid[size][size]) {
	int r = 0, c = 0;
	for (; r < size; ++r) {
		for (c = 0; c < size; ++c) {
			State state = grid[r][c].state;
			if (mode == PRINT) {
				printf("%c", state);
			} else if (mode == OVERLAY) {
				set_cur_pos(r + 1, c);
				put(state);
			} 
		}
		if (mode == PRINT) {
			printf("\n");
		}
	}
	print_stats();
	if (mode == OVERLAY) set_cur_pos(size + 3, 0);
}

bool is_in_range(int opt, int value) {
	int index, lo_lmt, hi_lmt;
	for (index = 1; index < NUM_OPTIONS; ++index)
		if (opt == options[index]) break;
	lo_lmt = opt_limits[index][0];
	hi_lmt = opt_limits[index][1];
	return lo_lmt < value && value < hi_lmt;
}

void shuffle(Cell grid[size][size]) {
	srandom(RAND_SEED);
	int r = 0, c, n = 0, num_cells = size * size;
	Cell grid_arr[num_cells];
	for (; r < size; ++r) {
		for (c = 0; c < size; ++c) {
			grid_arr[(size * r) + c] = grid[r][c];
		}
	}
	
	for (int index1 = num_cells - 1; index1 > 0; --index1) {
		int index2 = random() % (index1 + 1);
		State 
			s1 = (grid_arr + index1)->state,
			s2 = (grid_arr + index2)->state;
		(grid_arr + index1)->state = s2;
		(grid_arr + index2)->state = s1;
	}

	for (r = 0; r < size; ++r) {
		for (c = 0; c < size; ++c, ++n)
			grid[r][c] = grid_arr[n];
	}
}

void init(Cell grid[size][size], int updates_to_extinguish[size][size], int empty, int live) {
	int r = 0, c, n = 0;
	State nth_state;
	for (; r < size; ++r) {
		for (c = 0; c < size; ++c, ++n) {
			nth_state =
				n < empty ? EMPTY :
				n < empty + live ? LIVING :
				BURNING;
			Cell nth_cell = {r, c, nth_state};
			grid[r][c] = nth_cell;
		}
	}
	shuffle(grid);
	for (r = 0; r < size; ++r) {
		for (c = 0; c < size; ++c) {
			State s = grid[r][c].state;
			updates_to_extinguish[r][c] = 
				s == EMPTY ? -1 : 
				s == LIVING ? 4 : 
				3;
		}
	}
}

bool is_in_grid(int r, int c) {
	return (0 <= r && r < size) && (0 <= c && c < size);
}

bool spread(Cell grid[size][size], Cell tree) {
	if (pNeighbor == 0.00) return roll_success(pCatch);
	int
		r = tree.r, c = tree.c,
		r_addend = -1, c_addend,
		num_nbr = 0, num_fire_nbr = 0,
		nbr_r, nbr_c;
	Cell nbr;
	for (; r_addend <= 1; ++r_addend) {
		for (c_addend = -1; c_addend <= 1; ++c_addend) {
			if (r_addend == 0 && c_addend == 0) continue;
			else {
				nbr_r = r + r_addend;
				nbr_c = c + c_addend;
				if (is_in_grid(nbr_r, nbr_c)) {
					++num_nbr;
					nbr = grid[nbr_r][nbr_c];
					num_fire_nbr += nbr.state == BURNING;
					if (pNeighbor == 1.00) 
						if (num_fire_nbr < num_nbr)
							return false;
				}
			}
		}
	}
	double burning_ratio = (double) num_fire_nbr / num_nbr;
	if (burning_ratio < pNeighbor) return false;
	return roll_success(pCatch);
}

bool is_burned_out(Cell grid[size][size]) {
	for (int r = 0; r < size; ++r) {
		for (int c = 0; c < size; ++c) {
			if (grid[r][c].state == BURNING)
				return false;
		}
	}
	return true;
}

void update(Cell grid[size][size], int updates_to_extinguish[size][size]) {
	changes = 0;
	Cell temp_grid[size][size];
	int r = 0, c;
	for (; r < size; ++r) {
		for (c = 0; c < size; ++c) {
			Cell cell = grid[r][c];
			State state = cell.state;
			if (updates_to_extinguish == -1) {
				temp_grid[r][c] = cell;
			} else if (state == LIVING) {
				if (spread(grid, cell)) {
					cell.state = BURNING;
					--updates_to_extinguish[r][c];
					++changes;
				}
			} else if (state == BURNING) {
				if (--updates_to_extinguish[r][c] == 0) {
					state = BURNED;
					++changes;
					cell.state = BURNED;
				}
			} else if (state == BURNED) 
				--updates_to_extinguish[r][c];
			temp_grid[r][c] = cell;
		}
	}
	cum_changes += changes;
	for (r = 0; r < size; ++r) {
		for (c = 0; c < size;  ++c) {
			Cell new_cell = temp_grid[r][c];
			State new_state = new_cell.state;
			grid[r][c] = new_cell;
		}
	}
}

void simulate(void) {
	int
		grid_area = size * size,
		num_trees = density * grid_area,
		num_burning = pBurning * num_trees,
		num_living = num_trees - num_burning,
		num_empty = grid_area - num_trees,
		updates_to_extinguish[size][size];

	Cell grid[size][size];
	init(grid, updates_to_extinguish, num_empty, num_living);
	if (mode == PRINT) print_header(num_updates);
	else if (mode == OVERLAY) clear();
	while (true) {
		if (mode == PRINT && cycle > num_updates) return;
		print_grid(grid);
		if (is_burned_out(grid)) {
			printf("Fires are out.\n");
			if (mode == OVERLAY) set_cur_pos(size + 4, 0);
			return;
		}
		update(grid, updates_to_extinguish);
		++cycle;
		if (mode == OVERLAY) usleep(SLEEP_TIME);
	}
}

int main(int argc, char *argv[]) {
	int opt, value;
	while ((opt = getopt(argc, argv, "Hb:c:d:n:p:s:")) != -1) {
		switch (opt) {
			case 'H':
				print_help();
				return PASS;
			case 'b':
				value = atoi(optarg);
				if (is_in_range(opt, value)) {
					pBurning = value / 100.0;
					break;
				} else {
					print_error(opt);
					return FAIL;
				}
			case 'c':
				value = atoi(optarg);                                                   
                                if (is_in_range(opt, value)) {                                          
                                        pCatch = value / 100.0;                                       
                                        break;                                                                                          
								} else {                                                                
                                        print_error(opt);                                               
                                        return FAIL;                                                                                    
								}
			case 'd':
				value = atoi(optarg);
                                if (is_in_range(opt, value)) {
                                        density = value / 100.0;
                                        break;                                                                                          
								} else {
                                        print_error(opt);
                                        return FAIL;
                                }
			case 'n':
				value = atoi(optarg);
                                if (is_in_range(opt, value)) {
                                        pNeighbor = value / 100.0;
                                        break;                                                                                          
								} else {
                                        print_error(opt);
                                        return FAIL;
                                }
			case 'p':
				mode = PRINT;
				value = atoi(optarg);
                                if (is_in_range(opt, value)) {
                                        num_updates = value;
                                        break;                                                                                          
								} else {
                                        print_error(opt);
                                        return FAIL;
                                }
			case 's':
				value = atoi(optarg);
                                if (is_in_range(opt, value)) {
                                        size = value;
                                        break;                                                                                          
								} else {
                                        print_error(opt);
                                        return FAIL;
                               }
			default:
				print_error(opt);
				return FAIL;
		}
	}
	simulate();
	return PASS;
}
