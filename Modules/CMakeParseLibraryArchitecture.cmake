# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_policy(PUSH)
cmake_policy(SET CMP0053 NEW)
cmake_policy(SET CMP0054 NEW)

# Function parse implicit linker options.
# This is used internally by CMake and should not be included by user
# code.

function(cmake_parse_library_architecture implicit_dirs output_var)
  unset(library_arch)
  # Detect library architecture directory name.
  if(CMAKE_LIBRARY_ARCHITECTURE_REGEX)
    foreach(dir ${implicit_dirs})
      if("${dir}" MATCHES "/lib/${CMAKE_LIBRARY_ARCHITECTURE_REGEX}$")
        get_filename_component(arch "${dir}" NAME)
        set(library_arch "${arch}")
        break()
      endif()
    endforeach()
  endif()

  if(CMAKE_LIBRARY_ARCHITECTURE_REGEX_VERSIONED AND NOT library_arch)
    foreach(dir ${implicit_dirs})
      if("${dir}" MATCHES "/${CMAKE_LIBRARY_ARCHITECTURE_REGEX_VERSIONED}$")
        get_filename_component(arch "${dir}" DIRECTORY)
        get_filename_component(arch "${arch}" NAME)
        set(library_arch "${arch}")
        break()
      endif()
    endforeach()
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL QCC)
    foreach(dir ${implicit_dirs})
      if (dir MATCHES "/lib$")
        get_filename_component(assumedArchDir "${dir}" DIRECTORY)
        get_filename_component(archParentDir "${assumedArchDir}" DIRECTORY)
        if (archParentDir STREQUAL CMAKE_SYSROOT)
          get_filename_component(archDirName "${assumedArchDir}" NAME)
          set(library_arch "${archDirName}")
          break()
        endif()
      endif()
    endforeach()
  endif()

  # Return results.
  if(library_arch)
    set(${output_var} "${library_arch}" PARENT_SCOPE)
  endif()
endfunction()

cmake_policy(POP)
