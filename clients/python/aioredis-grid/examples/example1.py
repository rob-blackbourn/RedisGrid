import asyncio
import aioredis
from aioredisgrid import Redis

async def main():
    redis = await aioredis.create_redis('redis://localhost', commands_factory=Redis, encoding='UTF-8')

    await redis.grid_dim("a1", 2, 3, 1, 2, 3, 4, 5, 6)
    values = await redis.grid_dump("a1")
    print(values)

    redis.close()
    await redis.wait_closed()

if __name__ == '__main__':
    asyncio.get_event_loop().run_until_complete(main())
