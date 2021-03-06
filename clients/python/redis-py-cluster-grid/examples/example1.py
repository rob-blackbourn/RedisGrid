from redisclustergrid import StrictRedisCluster

host = "localhost"

startup_nodes = [
    {"host": host, "port": "7000"},
    {"host": host, "port": "7001"},
    {"host": host, "port": "7002"},
    {"host": host, "port": "7003"},
    {"host": host, "port": "7004"},
    {"host": host, "port": "7005"},
]

r = StrictRedisCluster(startup_nodes=startup_nodes, decode_responses=True)

r.grid_dim("a1", 2, 3, 1, 2, 3, 4, 5, 6)
values = r.grid_dump("a1")
print(values)
