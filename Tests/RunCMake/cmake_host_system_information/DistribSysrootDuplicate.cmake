# FROM_SYSROOT may be given at most once.
cmake_host_system_information(RESULT r QUERY FROM_SYSROOT ON FROM_SYSROOT OFF DISTRIB_ID)
