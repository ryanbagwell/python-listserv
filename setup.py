#!/usr/bin/env python
from distutils.core import setup, Extension

lcmdx = Extension('lcmdx', sources=['src/lcmdx.c'])

setup(
    name = "python-listsrv",
    version = "0.1",
    author = "Ryan Bagwell",
    author_email = "ryan@ryanbagwell.com",
    description = ("A Python interface for the Litserv mailing list manager"),
    license = "MIT",
    keywords = "Insteon automation",
    url = "https://github.com/ryanbagwell/python-listserv",
    packages=['listserv',],
    install_requires = [],
    scripts=['bin/lcmdx'],
    #ext_modules = [lcmdx],
)
