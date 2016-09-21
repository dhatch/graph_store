# -*- mode: python -*-
#
AddOption(
    '--dbg',
    action='store_true',
    help='debug build',
    default=False)

env = Environment(CPPPATH='#/src', CXXFLAGS=["--std=c++11"])

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
