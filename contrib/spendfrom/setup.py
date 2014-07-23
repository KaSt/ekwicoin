from distutils.core import setup
setup(name='ekwspendfrom',
      version='1.0',
      description='Command-line utility for ekwicoin "coin control"',
      author='Gavin Andresen',
      author_email='gavin@ekwicoinfoundation.org',
      requires=['jsonrpc'],
      scripts=['spendfrom.py'],
      )
