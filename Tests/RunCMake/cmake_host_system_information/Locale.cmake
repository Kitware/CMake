cmake_host_system_information(RESULT charset QUERY LOCALE_CHARSET)
message(STATUS "LOCALE_CHARSET='${charset}'")
