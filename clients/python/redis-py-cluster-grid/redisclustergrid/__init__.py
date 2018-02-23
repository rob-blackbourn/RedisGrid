"""Add support for the redis cluster grid
"""

try:
    import pandas
    from redisclustergrid.dataframeclient import StrictRedisCluster
except:
    from redisclustergrid.gridclient import StrictRedisCluster
