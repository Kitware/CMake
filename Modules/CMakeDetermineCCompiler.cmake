# determine the compiler to use for C programs
# NOTE, a generator may set CMAKE_C_COMPILER before
# loading this file to force a compiler.

FIND_PROGRAM(CMAKE_C_COMPILER NAMES $ENV{CC} gcc cc cl bcc PATHS /bin /usr/bin /usr/local/bin )
