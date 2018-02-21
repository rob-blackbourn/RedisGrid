"""A Pandas DataFrame mixin for the grid module
"""

import pandas as pd
from aioredis.util import _NOTSET
from aioredisgrid.grid import wait_make_grid

async def wait_make_dataframe(fut):
    unpacked = await wait_make_grid(fut)
    items = [(sub_list[0], [item for item in sub_list[2:]]) for sub_list in unpacked]
    df = pd.DataFrame.from_items(items)
    dtypes = dict((sub_list[0], sub_list[1]) for sub_list in unpacked)
    return df.astype(dtypes)    

def _encode(value):
    if isinstance(value, pd.Timestamp):
        return value.strftime("%Y-%m-%dT%H:%M:%S")
    elif isinstance(value, pd.Timedelta):
        return value.total_seconds()
    else:
        return str(value)

class DataFrameCommandsMixin:
    """DataFrame commands mixin
    """

    def grid_save_df(self, key, df):
        """Save the dataframe
        """
        columns, rows = df.shape
        values = [(name, series.dtype.name, *series.tolist()) for name, series in df.iteritems()]
        columns += 2
        values = [_encode(x) for sublist in values for x in sublist]
        return self.execute(b'GRID.DIM', key, rows, columns, *values)
        
    
    def grid_load_df(self, key, *, encoding=_NOTSET):
        """Load a DataFrame
        """
        fut = self.execute(b"GRID.DUMP", key, encoding=encoding)
        return wait_make_dataframe(fut)
