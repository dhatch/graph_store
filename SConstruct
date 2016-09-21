# -*- mode: python -*-

env = Environment(CPPPATH='#/src', CXXFLAGS="--std=c++11 -O0 -g")
Export('env')

env.SConscript(dirs=[
    'src'
], variant_dir='build')
