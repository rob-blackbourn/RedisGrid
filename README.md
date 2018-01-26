# RedisGrid

A grid module for Redis

## Status

This is definately alpha. Use at you're own risk.

## Description

This module provides a type which can hold a grid of data indexed by row and column.

## Commands

### GRID.DIM - dimension a new grid

    GRID.DIM <key> <rows> <columns> [r0c0, ... rNcN]

* key - key name for the rid
* rows - the number of rows in the grid
* columns - the number of columns in the grid

Optional args:

* the values for the grid to hold

If the rows or columns are 0 the grid will be deleted from the cache.

#### Examples

This will create a 2 row and 3 column grid populated with the given values.

    > GRID.DIM mygrid 2 3 1 2 3 4 5 6
    OK

This will shirnk the grid to 2 rows and 2 columns.

    > GRID.DIM mygrid 2 2
    OK

This will expand the grid to 3 rows and 4 columns.

    > GRID.DIM mygrid 3 4
    OK

This will delete the grid

    > GRID.DIM mygrid 0 0
    OK

### GRID.RANGE - return a range of data from a grid

    GRID.RANGE <key> <row-start> <row-end> <column-start> <column-end>

* key - key name for the grid
* row-start - the start row in the grid
* row-end - the end row in the grid
* column-start - the start column in the grid
* column-end - the end column in the grid

The ranges follow the standard redis convention where -1 is the end of the range.

#### Examples

This will return the entire grid

    > GRID.DIM mygrid 2 3 1 2 3 4 5 6
    OK
    > GRID.RANGE foo 0 -1 0 -1
    1) 1
    2) 2
    3) 3
    4) 4
    5) 5
    6) 6

This will returns the range in reverse order.

    > GRID.RANGE foo -1 0 -1 0
    1) 6
    2) 5
    3) 4
    4) 3
    5) 2
    6) 1

This will return a portion of the grid.

    > GRID.RANGE foo 0 1 1 2
    1) 2
    2) 3
    3) 5
    4) 6

This will return a portion of the grid with the columns reversed.

    > GRID.RANGE foo 0 1 2 1
    1) 3
    2) 2
    3) 6
    4) 5

### GRID.SHAPE - return the shape of a grid

    GRID.SHAPE <key>

* key - key name for the grid

#### Examples

This will return the rows and columns in the grid.

    > GRID.DIM foo 2 3 1 2 3 4 5 6
    OK
    > GRID.SHAPE foo
    1) (integer) 2
    2) (integer) 3

### GRID.SET - set values in a grid

    GRID.SET <row-start> <row-end> <column-start> <column-end> { r0c0 .. rNcN }

* key - key name for the grid
* row-start - the start row in the grid
* row-end - the end row in the grid
* column-start - the start column in the grid
* column-end - the end column in the grid
* the values to set in the grid to hold

As with the GRID.RANGE command the ranges can use negative numbers for reverse indexing.

#### Examples

This example sets the last two columns of each row with the given values.

    > GRID.DIM foo 2 3 1 2 3 4 5 6
    OK
    > GRID.SET foo 0 -1 1 -1 a b c d
    OK
    > GRID.RANGE foo 0 -1 0 -1
    1) 1
    2) a
    3) b
    4) 4
    5) c
    6) d
