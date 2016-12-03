# -*- mode: python -*-
import os

AddOption(
    '--dbg',
    action='store_true',
    help='debug build',
    default=False)

env = Environment(CPPPATH=['#/src', '#/lib/mongoose', '#/lib/jsoncpp/dist/', '#/gen-cpp'],
                  CXXFLAGS=["--std=c++11"])

Export('env')

if GetOption('dbg'):
    env.Append(CXXFLAGS=['-g'])
    opt_flag = '-O0'
    variant_dir = 'build/debug'
else:
    opt_flag = '-O3'
    variant_dir = 'build/release'

env.Append(CXXFLAGS=[opt_flag])

env.Command(
    '#/gen-cpp/Replication.cpp',
    '#/src/net/repl.thrift',
    'thrift --gen cpp $SOURCE')


env.Command('#/lib/mongoose/libmongoose.a', [
    Glob('#/lib/mongoose/mongoose/*'),
    Glob('#/lib/mongoose/mongoose.*'),
  ],
  'cmake -JSONCPP_DIR=../jsoncpp -DHAS_JSONCPP=ON . && make',
  chdir=True)

env.Command('#/lib/jsoncpp/dist/json/json.h', [
        Glob('#/lib/jsoncpp/include/*'),
        Glob('#/lib/jsoncpp/src/*'),
    ],
    'python amalgamate.py',
    chdir='lib/jsoncpp')

env.Command('#/lib/jsoncpp/src/lib_json/libjsoncpp.a', [
        Glob('#/lib/jsoncpp/include/*'),
    ],
    'cmake . && make',
    chdir='lib/jsoncpp')


env.SConscript(dirs=[
    'src'
], variant_dir=variant_dir)
