# -*- mode: python -*-
import os

AddOption(
    '--dbg',
    action='store_true',
    help='debug build',
    default=False)

env = Environment(CPPPATH=['#/src', '#/lib'], CXXFLAGS=["--std=c++11"])

Export('env')

if GetOption('dbg'):
    env.Append(CXXFLAGS=['-g'])
    opt_flag = '-O0'
    variant_dir = 'build/debug'
else:
    opt_flag = '-O3'
    variant_dir = 'build/release'

env.Append(CXXFLAGS=[opt_flag])

env.SConscript(dirs=[
    'src'
], variant_dir=variant_dir)

env.Command('#/lib/mongoose/libmongoose.a', [
    Glob('#/lib/mongoose/mongoose/*'),
    Glob('#/lib/mongoose/mongoose.*'),
  ],
  'cmake -JSONCPP_DIR=../jsoncpp -DHAS_JSONCPP=ON . && make',
  chdir=True)
