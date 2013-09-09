#!/usr/bin/env python
from distutils.core import setup
from distutils import sysconfig
from distutils.command.install import install as Install
import os
import shutil


class PostInstall(Install):

    def run(self):
        Install.run(self)
        pre = sysconfig.get_config_var("prefix")
        dest = os.path.join(pre, "bin", "lcmdx")
        source = os.path.join(os.path.dirname(os.path.realpath(__file__)), "bin", "lcmdx")
        shutil.copy2(source, dest)


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
    cmdclass=dict(install=PostInstall),
)

