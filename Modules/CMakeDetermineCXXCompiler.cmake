# determine the compiler to use for C++ programs
# NOTE, a generator may set CMAKE_CXX_COMPILER before
# loading this file to force a compiler.

FIND_PROGRAM(CMAKE_CXX_COMPILER NAMES $ENV{CXX} c++ g++ CC aCC cl bcc PATHS /bin /usr/bin /usr/local/bin )
