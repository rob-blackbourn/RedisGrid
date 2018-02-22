from redisclustergrid import GridClusterClient

startup_nodes = [
    {"host": "127.0.0.1", "port": "7000"},
    {"host": "127.0.0.1", "port": "7001"},
    {"host": "127.0.0.1", "port": "7002"},
    {"host": "127.0.0.1", "port": "7003"},
    {"host": "127.0.0.1", "port": "7004"},
    {"host": "127.0.0.1", "port": "7005"},
    ]

r = GridClusterClient(startup_nodes=startup_nodes, decode_responses=True)

r.grid_dim("a1", 2, 3, 1, 2, 3, 4, 5, 6)
values = r.grid_dump("a1")
print(values)
