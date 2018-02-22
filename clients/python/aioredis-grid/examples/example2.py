import asyncio
import aioredis
from aioredisgrid import Redis
import pandas as pd

async def main():
    redis = await aioredis.create_redis('redis://localhost', commands_factory=Redis, encoding='UTF-8')

    df = pd.DataFrame({'col1': [1, 2], 'col2': [3, 4]})
    await redis.grid_save_df('df', df)
    df2 = await redis.grid_load_df('df')
    print(df2)

    redis.close()
    await redis.wait_closed()

if __name__ == '__main__':
    asyncio.get_event_loop().run_until_complete(main())
