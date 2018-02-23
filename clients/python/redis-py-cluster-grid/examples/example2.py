from redisclustergrid import StrictRedisCluster
import pandas as pd

host = "10.11.153.125"
startup_nodes = [
    {"host": host, "port": "7000"},
    {"host": host, "port": "7001"},
    {"host": host, "port": "7002"},
    {"host": host, "port": "7003"},
    {"host": host, "port": "7004"},
    {"host": host, "port": "7005"},
    ]

r = StrictRedisCluster(startup_nodes=startup_nodes, decode_responses=True)

df = pd.DataFrame({'col1': [1, 2], 'col2': [3, 4]})
r.grid_save_df('df', df)
df2 = r.grid_load_df('df')
print(df2)
