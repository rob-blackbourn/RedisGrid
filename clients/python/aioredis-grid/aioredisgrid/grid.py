"""Support for the RedisGrid module
"""

from aioredis.util import _NOTSET

async def wait_make_grid(fut):
    """Transform a grid dump into a list of lists"""
    res = await fut
    if res in (b'QUEUED', 'QUEUED'):
        return res
    _, columns = res[:2]
    unpacked = [res[x:x+columns] for x in range(2, len(res), columns)]
    return unpacked

class GridCommandsMixin:
    """
    Support for commands provided by the RedisGrid module.
    """

    def grid_dim(self, key, rows, columns, *values):
        """Dimension a grid, and optionally populate it's values

        :raises TypeError: if rows or columns is not set
        """
        if not isinstance(rows, int):
            raise TypeError("rows argument must be int")
        if not isinstance(columns, int):
            raise TypeError("columns argument must be int")
        return self.execute(b'GRID.DIM', key, rows, columns, *values)
        
    def grid_range(self, key, row_start, row_end, column_start, column_end, *, encoding=_NOTSET):
        """Returns the specified elements of the grid stored at key

        :raises TypeError: if row_start, row_end, column_start or column_end is not set
        """
        if not isinstance(row_start, int):
            raise TypeError("row_start argument must be int")
        if not isinstance(row_end, int):
            raise TypeError("row_end argument must be int")
        if not isinstance(column_start, int):
            raise TypeError("column_start argument must be int")
        if not isinstance(column_end, int):
            raise TypeError("column_end argument must be int")
        return self.execute(b'GRID.RANGE', key, row_start, row_end, column_start, column_end, encoding=encoding)
    
    def grid_shape(self, key):
        """Returns a tuple (rows,columns) of the grid stored at key.
        """
        return self.execute(b"GRID.SHAPE", key)

    def grid_set(self, key, row_start, row_end, column_start, column_end, *values):
        """Sets the values in a grid stored at key using the supplied range arguments.

        :raises TypeError: if row_start, row_end, column_start or column_end is not set
        """
        if not isinstance(row_start, int):
            raise TypeError("row_start argument must be int")
        if not isinstance(row_end, int):
            raise TypeError("row_end argument must be int")
        if not isinstance(column_start, int):
            raise TypeError("column_start argument must be int")
        if not isinstance(column_end, int):
            raise TypeError("column_end argument must be int")
        return self.execute(b"GRID.SET", key, row_start, row_end, column_start, column_end, *values)
   
    def grid_dump(self, key):
        """Returns the entire grid stored at key.
        """
        fut = self.execute(b"GRID.DUMP", key)   
        return wait_make_grid(fut)

