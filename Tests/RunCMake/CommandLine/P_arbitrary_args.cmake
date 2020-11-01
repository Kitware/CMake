if(NOT ("${CMAKE_ARGV3}" STREQUAL "--" AND "${CMAKE_ARGV4}" STREQUAL "-DFOO"))
    message(FATAL_ERROR "`-DFOO` shouldn't trigger an error after `--`")
endif()
