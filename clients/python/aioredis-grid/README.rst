aioredisgrid
==========================

A python module for the Redis grid module using `aioredis <https://github.com/aio-libs/aioredis>`_.

Installation
------------

To install aioredis-grid, simply:

.. code-block:: bash

    $ python setup.py install

If you require support for pandas dataframes follow the the instructions at `scipy <https://www.scipy.org/>`_.

Usage
-----

An example of grid usage is:

.. code-block:: python

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

    # Will print
    # [['1', '2', '3'], ['4', '5', '6']]

An example of data frame usage is :

.. code-block:: python

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
    
    # Will print
    #    col1  col2
    # 0     1     3
    # 1     2     4

