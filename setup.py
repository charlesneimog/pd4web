from setuptools import setup
from setuptools.dist import Distribution

class BinaryDistribution(Distribution):
    def has_ext_modules(self):
        return True  # força wheel binário (não "py3-none-any")

setup(distclass=BinaryDistribution)

