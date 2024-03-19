# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

if(glob)
  list(TRANSFORM glob PREPEND "${dir}/")
  file(GLOB files RELATIVE "${dir}" ${glob})
  foreach(file IN LISTS files)
    message(STATUS "${file}")
    if(verify)
      execute_process(COMMAND ${verify} ${file} COMMAND_ERROR_IS_FATAL ANY)
    endif()
  endforeach()
  message(STATUS "")
endif()
