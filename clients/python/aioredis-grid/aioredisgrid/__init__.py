"""Add support for the RedisGrid module
"""

from aioredis import Redis as _Redis
from aioredisgrid.grid import GridCommandsMixin

__all__ = [ 'Redis' ]

try:
    from aioredisgrid.dataframe import DataFrameCommandsMixin
    class Redis(_Redis, GridCommandsMixin, DataFrameCommandsMixin):
        """Add support for The RedisGrid module
        """
except:
    class Redis(_Redis, GridCommandsMixin):
        """Add support for The RedisGrid module
        """
