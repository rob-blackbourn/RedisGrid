from redisgrid import  GridClient

client = GridClient(host='localhost')

client.grid_dim("a1", 2, 3, 1, 2, 3, 4, 5, 6)
values = client.grid_dump("a1")
print(values)
