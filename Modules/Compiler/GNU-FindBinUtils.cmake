if(NOT DEFINED _CMAKE_PROCESSING_LANGUAGE OR _CMAKE_PROCESSING_LANGUAGE STREQUAL "")
  message(FATAL_ERROR "Internal error: _CMAKE_PROCESSING_LANGUAGE is not set")
endif()

string(REGEX MATCH "^([0-9]+\\.[0-9]+)" __version_x_y
    "${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_VERSION}")

# Try to find tools in the same directory as GCC itself
get_filename_component(__gcc_hints "${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER}" DIRECTORY)

# http://manpages.ubuntu.com/manpages/wily/en/man1/gcc-ar.1.html
find_program(CMAKE_GCC_AR NAMES
    "${_CMAKE_TOOLCHAIN_PREFIX}gcc-ar"
    "${_CMAKE_TOOLCHAIN_PREFIX}gcc-ar-${__version_x_y}"
    HINTS ${__gcc_hints}
    DOC "A wrapper around 'ar' adding the appropriate '--plugin' option for the GCC compiler"
)

# http://manpages.ubuntu.com/manpages/wily/en/man1/gcc-ranlib.1.html
find_program(CMAKE_GCC_RANLIB NAMES
    "${_CMAKE_TOOLCHAIN_PREFIX}gcc-ranlib"
    "${_CMAKE_TOOLCHAIN_PREFIX}gcc-ranlib-${__version_x_y}"
    HINTS ${__gcc_hints}
    DOC "A wrapper around 'ranlib' adding the appropriate '--plugin' option for the GCC compiler"
)
