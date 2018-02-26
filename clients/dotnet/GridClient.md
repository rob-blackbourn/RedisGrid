# GridClient

## Redis.RedisGrid
            
An extension class providing methods for the Redis Grid module.
        
### Methods

#### bool GridDim(this IDatabase db, RedisKey key, string[,] grid)
Dimension a grid with a two dimensional array.
##### Parameters
* **db:** The database in which the grid will be stored.
* **key:** The key with which the grid is associated.
* **grid:** The grid to store against the key
##### Return value
If the grid is stored true, otherwise false.

#### Task<bool> GridDimAsync(this IDatabase db, RedisKey key, string[,])
Dimension a grid asynchronously with a two dimensional array.
##### Parameters
* **db:** The database in which the grid will be stored.
* **key:** The key with which the grid is associated.
* **grid:** The grid to store against the key
##### Return value
If the grid is stored true, otherwise false.

#### bool GridDim(this IDatabase db, RedisKey key, int rows, int columns, params string[] elements)
Dimension a grid of a given size with an optional one dimentional array of elements.
##### Parameters
* **db:** The database in which the grid will be stored.
* **key:** The key with which the grid is associated.
* **rows:** The number of rows in the grid
* **columns:** The number of columns in the grid
* **elements:** An optional one dimensional array of the elements of the grid presented row-wise.
##### Return value
If the grid is stored true, otherwise false.

#### Task<bool> GridDimAsync(this IDatabase db, RedisKey key, int rows, int rows, string[])
Dimension a grid asynchronously of a given size with an optional one dimentional array of elements.
##### Parameters
* **db:** The database in which the grid will be stored.
* **key:** The key with which the grid is associated.
* **rows:** The number of rows in the grid
* **columns:** The number of columns in the grid
* **elements:** An optional one dimensional array of the elements of the grid presented row-wise.
##### Return value
If the grid is stored true, otherwise false.

#### int[] GridShape(this IDatabase db, RedisKey key)
Find the number of rows and columns in a grid.
##### Parameters
* **db:** The database in which the grid will be stored.
* **key:** The key against which the grid is associated.
##### Return value
An array of two integers where the first is the rows and the second the columns.

#### Task<int[]> GridShapeAsync(this IDatabase db, RedisKey key)
Find the number of rows and columns in a grid asynchronously.
##### Parameters
* **db:** The database in which the grid is stored.
* **key:** The key against which the grid is associated.
##### Return value
An array of two integers where the first is the rows and the second the columns.

#### RedisValue[] GridRange(this IDatabase db, RedisKey key, int rowStart, int rowEnd, int columnStart, int columnEnd)
Query a range of rows and columns in a grid. If the start row or column is less than the end row or column the results will obey the direction.
##### Parameters
* **db:** The database in which the grid is stored.
* **key:** The key against which the grid is associated.
* **rowStart:** The first row where 0 is the first element and -1 is the last.
* **rowEnd:** The last row where 0 is the first element and -1 is the last.
* **columnStart:** The first column where 0 is the first element and -1 is the last.
* **columnEnd:** The last column where 0 is the first element and -1 is the last.
##### Return value
A one dimentional array of the range ordered row-wise.

#### Task<RedisValue[]> GridRangeAsync(this IDatabase db, RedisKey key, int rowStart, int rowEnd, int columnStart, int columnEnd)
Query a range of rows and columns in a grid asynchronously. If the start row or column is less than the end row or column the results will obey the direction.
##### Parameters
* **db:** The database in which the grid is stored.
* **key:** The key against which the grid is associated.
* **rowStart:** The first row where 0 is the first element and -1 is the last.
* **rowEnd:** The last row where 0 is the first element and -1 is the last.
* **columnStart:** The first column where 0 is the first element and -1 is the last.
* **columnEnd:** The last column where 0 is the first element and -1 is the last.
##### Return value
A one dimentional array of the range ordered row-wise.

#### bool GridSet(this IDatabase db, RedisKey key, int rowStart, int columnStart, string[,] grid)
Set a range of values in a grid.
##### Parameters
* **db:** The database in which the grid is stored.
* **key:** The key against which the grid is associated.
* **rowStart:** The row offset at which the data should be set.
* **columnStart:** The column offset at which the data should be set.
* **grid:** A two dimentional array of items to set in the.
##### Return value
The method returns true if the operation succeeded, otheraise false.

#### Task<bool> GridSetAsync(this IDatabase db, RedisKey key, int rowStart, int columnStart, string[,] grid)
Set a range of values in a grid asynchronously.
##### Parameters
* **db:** The database in which the grid is stored.
* **key:** The key against which the grid is associated.
* **rowStart:** The row offset at which the data should be set.
* **columnStart:** The column offset at which the data should be set.
* **grid:** A two dimentional array of items to set in the.
##### Return value
The method returns true if the operation succeeded, otheraise false.

#### bool GridSet(this IDatabase db, RedisKey key, int rowStart, int rowEnd, int columnStart, int columnEnd, string[] elements)
Set values in a grid between row and column limits with a one dimentional array of data. This method reflects the underlying process of storing data in the grid.
##### Parameters
* **db:** The database in which the grid is stored.
* **key:** The key against which the grid is associated.
* **rowStart:** The first row where 0 is the first element and -1 is the last.
* **rowEnd:** The last row where 0 is the first element and -1 is the last.
* **columnStart:** The first column where 0 is the first element and -1 is the last.
* **columnEnd:** The last column where 0 is the first element and -1 is the last.
* **elements:** An one dimensional array of the elements to set in the grid presented row-wise.
##### Return value
The method returns true if the operation succeeded, otheraise false.

#### Task<bool> GridSetAsync(this IDatabase db, RedisKey key, int rowStart, int rowEnd, int columnStart, int columnEnd, string[] elements)
Set values asynchronously in a grid between row and column limits with a one dimentional array of data. This method reflects the underlying process of storing data in the grid.
##### Parameters
* **db:** The database in which the grid is stored.
* **key:** The key against which the grid is associated.
* **rowStart:** The first row where 0 is the first element and -1 is the last.
* **rowEnd:** The last row where 0 is the first element and -1 is the last.
* **columnStart:** The first column where 0 is the first element and -1 is the last.
* **columnEnd:** The last column where 0 is the first element and -1 is the last.
* **elements:** An one dimensional array of the elements to set in the grid presented row-wise.
##### Return value
The method returns true if the operation succeeded, otheraise false.

#### RedisValue[,] GridDump(this IDatabase db, RedisKey key)
Returns the grid found with the given key.
##### Parameters
* **db:** The database in which the grid is stored.
* **key:** The key against which the grid is associated.
##### Return value
The two dimentional grid found.

#### Task<RedisValue[,]> GridDumpAsync(this IDatabase db, RedisKey key)
Returns the grid found with the given key asynchronously.
##### Parameters
* **db:** The database in which the grid is stored.
* **key:** The key against which the grid is associated.
##### Return value
The two dimentional grid found.

#### string[,] AsStringGrid(this RedisValue[,] source)
Convert a two dimentional array of RedisValue objects to an array of the same size of strings. Note that the underlying storage of the grid is a string or null.
##### Parameters
* **source:** The source grid.
##### Return value
A two dimentional string array.

#### string[,] AsStringGrid(this RedisValue[] source, int rows, int columns)
Convert a one dimensional array of a given shape to a two dimensional string array.
##### Parameters
* **source:** A one dimensional array of RedisValue structs ordered row-wise.
* **rows:** The number of rows in the grid.
* **columns:** The number of columns in the grid.
##### Return value
The input converted to a two dimensional string array.

#### DataFrame AsDataFrame(this RedisValue[,] source)
Convert a two dimensional array of RedisStructs to a data frame. The grid must have been saved with correctly formatted information regarding the name and type of each series in the data frame.
##### Parameters
* **source:** A two dimentional array of RedisValue structs.
##### Return value
A data frame

#### string[,] AsStringGrid(this DataFrame dataFrame)
Convert a data frame into a two dimensional array of strings. The grid returned includes the name and type of each series in the data frame.
##### Parameters
* **dataFrame:** The data frame to convert.
##### Return value
A two dimensional array of strings.