#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "histogram_grid.h"

//
// Certainty grid-related functions.
//

grid_t * grid_init(int dimension, int resolution) {
	int i, j;

	grid_t * grid;
	grid = (grid_t *)malloc(sizeof(grid_t));

	if (NULL == grid) return NULL;

  // TODO: Also `assert` that the resolution is within some reasonable values (???).
  assert(dimension % 2 == 1);

	/* Initialize grid's parameters. Guarantee that dimension is odd. */
	grid->dimension = dimension;
	grid->resolution = resolution;

	/*
	** Allocate enough memory for the grid (dimension x dimension ints).
	**
	** Making this a single allocation is simpler. Also, This *IS* a performance
	** hack. Use [i * dim + j] to iterate over it.
	*/
	grid->cells = (int *)malloc(dimension * dimension * sizeof(int));

	if (NULL == grid->cells) return NULL;

  // Initial value, C_0 = 0.
	for (i = 0; i < dimension; ++i) {
		for (j = 0; j < dimension; ++j) {
			grid->cells[i * dimension + j] = 0;
		}
	}

	return grid;
}

int grid_update(grid_t * grid, int pos_x, int pos_y, range_measure_t data) {

	if (grid == NULL) return 0;
	if (grid->cells == NULL) return 0;

	int new_x, new_y;

	/*
	** Transform each sensor reading into cartesian coordinates and increase the
	** corresponding cell's obstacle density.
	** ** Polar to cartesian:
	** (r, o) -> (r * cos(x), r * sin(y))
	**
	** Remember that cos() and sin() expect angles in RADIANS, not DEGREES.
	*/
	/* Initialize the offset of the point i. */
	new_x = pos_x;
	new_y = pos_y;

	new_x += (int) floor((data.distance / grid->resolution) *
		cos(data.direction * M_PI / 180));

	new_y += (int) floor((data.distance / grid->resolution) *
		sin(data.direction * M_PI / 180));

	/* Is this point inside the grid? (to avoid overflows) */
	if (new_x < grid->dimension && new_y < grid->dimension) {
		grid->cells[new_x * grid->dimension + new_y] += 1;
	}

	return 1;
}

/* TODO: Finish implementing get_moving_window. */
grid_t * get_moving_window(grid_t * grid, int current_position_x,
		int current_position_y, int dim) {

	int i, j; /* Indexes for the moving window. */
	int grid_i, grid_j; /* Indexes for the grid. */
	grid_t * moving_window;

	/*
	** Create a window with dimension dim and the same resolution as grid.
	**
	** If grid_init returns NULL, exit the function.
	*/
	moving_window = grid_init(dim, grid->resolution);

	if (NULL != moving_window) {

		/* Populate moving_window's cells with the values of the ones in grid. */
		for (i = 0; i < dim; ++i) {
			for (j = 0; j < dim; ++j) {

				/* x and y are the center coordinates of the body with sensors. */
				grid_i = i + current_position_x + (dim - 1) / 2;
				grid_j = j + current_position_y + (dim - 1) / 2;

				/* Copy the information from the grid to the moving window. */
				if (grid_i < grid->dimension && grid_j < grid->dimension) {
					moving_window->cells[i * dim + j] = grid->cells[grid_i *
						grid->dimension + grid_j];
				}
			}
		}
	}

	return moving_window;
}

