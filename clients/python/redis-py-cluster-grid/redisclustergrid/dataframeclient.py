import pandas as pd
import redisclustergrid.gridclient as gridclient

def _encode(value):
    if isinstance(value, pd.Timestamp):
        return value.strftime("%Y-%m-%dT%H:%M:%S")
    elif isinstance(value, pd.Timedelta):
        return value.total_seconds()
    else:
        return str(value)

class StrictRedisCluster(gridclient.StrictRedisCluster):
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def grid_save_df(self, key, df):
        columns, rows = df.shape
        values = [(name, series.dtype.name, *series.tolist()) for name, series in df.iteritems()]
        columns += 2
        flat = [_encode(x) for sublist in values for x in sublist]
        return self.execute_command("GRID.DIM", key, rows, columns, *flat)
    
    def grid_load_df(self, key):
        response = self.grid_dump(key)
        items = [(sub_list[0], [item for item in sub_list[2:]]) for sub_list in response]
        df= pd.DataFrame.from_items(items)
        dtypes = dict((sub_list[0], sub_list[1]) for sub_list in response)
        return df.astype(dtypes)
