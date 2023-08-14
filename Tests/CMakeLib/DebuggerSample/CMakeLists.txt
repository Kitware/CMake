cmake_minimum_required(VERSION 3.26)
project(DebuggerSample NONE)
message("Hello CMake Debugger")

# There are concerns that because the debugger uses libuv for pipe
# communication, libuv may register a SIGCHILD handler that interferes with
# the existing handler used by kwsys process management. Test this case with a
# simple external process.
execute_process(COMMAND "${CMAKE_COMMAND}" -E echo test)
