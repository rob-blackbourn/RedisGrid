from redisgrid import StrictRedis

r = StrictRedis(host='localhost')

r.grid_dim("a1", 2, 3, 1, 2, 3, 4, 5, 6)
values = r.grid_dump("a1")
print(values)
