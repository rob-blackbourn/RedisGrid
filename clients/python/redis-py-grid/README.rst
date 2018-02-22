redisgrid
=========

A python module for the Redis grid module using redis-py `redis-py
<https://github.com/andymccurdy/redis-py>`

Installation
------------

To install redis-py-grid, simply:

.. code-block:: bash

    $ python setup.py install

If you require support for pandas dataframes follow the the instructions at `scipyre<https://www.scipy.org/>.

Usage
-----

The following example stores and retrieves a grid:

.. code-block:: pycon

    >>> from redisgrid import GridClient
    >>> r = GridClient(host='localhost')
    >>> r.grid_dim("a1", 2, 3, 1, 2, 3, 4, 5, 6)
    True
    >>> values = r.grid_dump("a1")
    >>> print(values)
    [[b'1', b'2', b'3'], [b'4', b'5', b'6']]

The following example stores and retrieves a pandas dataframe.

.. code-block:: pycon

    >>> from redisgrid import DataFrameClient
    >>> import pandas as pd
    >>> r = DataFrameClient(host="localhost")
    >>> df = pd.DataFrame({'col1': [1, 2], 'col2': [3, 4]})
    >>> r.grid_save_df('df', df)
    True
    >>> df2 = r.grid_load_df('df')
    >>> print(df2)
       col1  col2
    0     1     3
    1     2     4
