# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

macro(cmake_sycl_filter_implicit_libs variable)
  foreach(library IN LISTS ${variable})
    if(library MATCHES "^sycl([0-9]+d?|-devicelib-host)?([.]lib)?$")
      list(APPEND CMAKE_SYCL_RUNTIME_LIBRARY_LINK_OPTIONS_DEFAULT "${library}")
      list(REMOVE_ITEM ${variable} "${library}")
    endif()
  endforeach()
  if(CMAKE_SYCL_RUNTIME_LIBRARY_LINK_OPTIONS_DEFAULT)
    list(REMOVE_DUPLICATES CMAKE_SYCL_RUNTIME_LIBRARY_LINK_OPTIONS_DEFAULT)
    set(CMAKE_SYCL_RUNTIME_LIBRARY_DEFAULT DEFAULT)
  endif()
endmacro()
