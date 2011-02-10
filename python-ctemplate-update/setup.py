#!/usr/bin/python
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2007 Bastian Kleineidam
from distutils.core import setup, Extension

module1 = Extension('ctemplate',
                    sources = ['src/ctemplate.cpp'],
                    libraries = ["ctemplate", "pthread"])

myname = "Bastian Kleineidam"
myemail = "calvin@debian.org"

setup (name = 'python-ctemplate',
       version = '0.7',
       description = 'Python wrapper for the ctemplate library',
       license = "BSD, see COPYING",
       author = myname,
       author_email = myemail,
       maintainer = myname,
       maintainer_email = myemail,
       ext_modules = [module1])
