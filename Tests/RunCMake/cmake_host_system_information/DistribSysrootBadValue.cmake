# FROM_SYSROOT requires a boolean value.  A non-boolean is rejected, which also
# guards the "forgot the value, ate the next key" mistake.
cmake_host_system_information(RESULT r QUERY FROM_SYSROOT bogus DISTRIB_ID)
