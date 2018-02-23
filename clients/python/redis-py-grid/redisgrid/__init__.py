"""Add support for grids
"""

try:
    import pandas
    from redisgrid.dataframeclient import StrictRedis
except:
    from redisgrid.gridclient import StrictRedis

