cmake_diagnostic(SET CMD_DEPRECATED IGNORE)

# Note: We're using the deprecation warning from cmMakefile::SetPolicy as a
# test, which requires use of policies within the deprecation window (as of
# CMake 4.4, CMP0066...CMP0155). These will likely need to be rotated to newer
# policies when that window changes.

cmake_policy(SET CMP0218 NEW)
cmake_policy(SET CMP0152 OLD) # should be silent

cmake_policy(SET CMP0218 OLD)
cmake_policy(SET CMP0153 OLD) # should NOT be silent

set(CMAKE_ERROR_DEPRECATED ON)
message(DEPRECATION "Test") # should be error
