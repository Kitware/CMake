
cmake_minimum_required (VERSION 2.8.12)
project(Consumer)

foreach(_lang C CXX)
  set(_existing ${CMAKE_${_lang}_IMPLICIT_LINK_DIRECTORIES})
  set(CMAKE_${_lang}_IMPLICIT_LINK_DIRECTORIES)
  foreach(_dir ${_existing})
    if(_dir MATCHES ${CMAKE_SYSROOT})
      list(APPEND CMAKE_${_lang}_IMPLICIT_LINK_DIRECTORIES ${_dir})
    endif()
  endforeach()
endforeach()

find_package(ZLIB REQUIRED)
set(CMAKE_CXX_LINK_FLAGS "-L\"${CMAKE_SYSROOT}/usr/lib/${triple}\"")

add_executable(executable main.cpp)
target_link_libraries(executable ${ZLIB_LIBRARIES})

install(TARGETS executable DESTINATION bin)
