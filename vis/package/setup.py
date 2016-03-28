
from distutils.core import setup

__author__ = 'seanparadiso'

setup(
    name='citrine_challenge',
    version='0.1',
    description="Simple vis tool for evaluating challenge submissions",
    author="Sean Paradiso",
    author_email="sean@citrine.io",
    url="https://github.com/sparadiso/tetr/tree/master/vis",
    packages=['citrine_challenge'],
    requires=['scipy', 'mayavi', 'numpy'],
)
