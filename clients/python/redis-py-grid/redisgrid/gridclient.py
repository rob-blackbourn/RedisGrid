import six
import redis
from redis.client import bool_ok

def _parse_grid_dump(response, **options):
    _, columns = response[:2]
    unpacked = [response[x:x+columns] for x in range(2, len(response), columns)]
    return unpacked

class StrictRedis(redis.StrictRedis):
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        
        # Set the module commands' callbacks
        MODULE_CALLBACKS = {
                'GRID.DUMP': _parse_grid_dump,
                'GRID.DIM': bool_ok,
                'GRID.SET': bool_ok,
                "GRID.SHAPE": tuple
                }
        for k, v in six.iteritems(MODULE_CALLBACKS):
            self.set_response_callback(k, v)
    
    def grid_dim(self, key, rows, columns, *args):
        return self.execute_command("GRID.DIM", key, rows, columns, *args)
    
    def grid_range(self, key, row_start, row_end, column_start, column_end):
        return self.execute_command("GRID.RANGE", key, row_start, row_end, column_start, column_end)
    
    def grid_shape(self, key):
        return self.execute_command("GRID.SHAPE", key)
    
    def grid_set(self, key, row_start, row_end, column_start, column_end, *args):
        return self.execute_command("GRID.SET", key, row_start, row_end, column_start, column_end, *args)
    
    def grid_dump(self, key):
        return self.execute_command("GRID.DUMP", key)