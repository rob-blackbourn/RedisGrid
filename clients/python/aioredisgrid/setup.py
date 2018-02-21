"""The install script for aioredisgrid.
See:
https://github.com/rob-blackbourn/RedisGrid
"""

# Always prefer setuptools over distutils
from setuptools import setup, find_packages
# To use a consistent encoding
from codecs import open
from os import path

here = path.abspath(path.dirname(__file__))

# Get the long description from the README file
with open(path.join(here, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name='aioredisgrid',
    version='1.0.0',
    description='A client for the Redis grid module using aioredis',
    long_description=long_description,
    url='https://github.com/rob-blackbourn/RedisGrid/tree/master/clients/python/aioredisgrid',
    author='Rob Blackbourn',
    author_email='rob.blackbourn@gmail.com',
    classifiers=[  # Optional
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3 :: Only',
        'Topic :: Software Development',
        'Topic :: Software Development :: Libraries',
        'Framework :: AsyncIO',
    ],
    keywords='redis module grid aioredis asyncio',
    packages=find_packages(exclude=['tests', 'examples']),
    install_requires=['aioredis'],  # Optional
    extras_require={  # Optional
        'pandas': ['pandas'],
    },
)