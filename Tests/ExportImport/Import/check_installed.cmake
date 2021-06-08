cmake_minimum_required(VERSION 3.20)

function(check_installed expect)
  file(GLOB_RECURSE actual
    LIST_DIRECTORIES TRUE
    RELATIVE ${CMAKE_INSTALL_PREFIX}
    ${CMAKE_INSTALL_PREFIX}/*
    )
  if(actual)
    list(SORT actual)
  endif()
  if(NOT "${actual}" MATCHES "${expect}")
    message(FATAL_ERROR "Installed files:
  ${actual}
do not match what we expected:
  ${expect}
in directory:
  ${CMAKE_INSTALL_PREFIX}")
  endif()
endfunction()
