#!/usr/bin/env python

from distutils.core import setup
from spadic import __version__

setup(name='spadic',
      version=__version__,
      description='Susibo -> Spadic 1.0 control software',
      author='Michael Krieger',
      author_email='michael.krieger@ziti.uni-heidelberg.de',
      url='http://spadic.uni-hd.de',
      packages=['spadic',
                'spadic.control',
                'spadic.control.ui',
                'spadic.tools'],
      scripts=['scripts/spadic_control',
               'scripts/spadic_server',
               'scripts/spadic_scope',
               'scripts/spadic_recorder'],
     )

