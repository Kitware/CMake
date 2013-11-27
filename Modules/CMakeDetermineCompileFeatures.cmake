
#=============================================================================
# Copyright 2013 Stephen Kelly <steveire@gmail.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

function(cmake_determine_compile_features lang)

  if(lang STREQUAL CXX AND COMMAND cmake_record_cxx_compile_features)
    message(STATUS "Detecting ${lang} compile features")

    set(CMAKE_CXX98_COMPILE_EXTENSIONS)
    set(CMAKE_CXX11_COMPILE_FEATURES)
    set(CMAKE_CXX11_COMPILE_EXTENSIONS)

    include("${CMAKE_ROOT}/Modules/Internal/FeatureTesting.cmake")

    cmake_record_cxx_compile_features()

    if(NOT _result EQUAL 0)
      message(STATUS "Detecting ${lang} compile features - failed")
      return()
    endif()

    string(REPLACE "${CMAKE_CXX98_COMPILE_EXTENSIONS}" "" CMAKE_CXX11_COMPILE_EXTENSIONS "${CMAKE_CXX11_COMPILE_EXTENSIONS}")
    string(REPLACE "${CMAKE_CXX11_COMPILE_FEATURES}" "" CMAKE_CXX11_COMPILE_EXTENSIONS "${CMAKE_CXX11_COMPILE_EXTENSIONS}")

    if(NOT CMAKE_CXX_COMPILE_FEATURES)
      set(CMAKE_CXX_COMPILE_FEATURES
        ${CMAKE_CXX98_COMPILE_EXTENSIONS}
        ${CMAKE_CXX11_COMPILE_FEATURES}
        ${CMAKE_CXX11_COMPILE_EXTENSIONS}
      )
    endif()

    set(CMAKE_CXX_COMPILE_FEATURES ${CMAKE_CXX_COMPILE_FEATURES} PARENT_SCOPE)
    set(CMAKE_CXX98_COMPILE_EXTENSIONS ${CMAKE_CXX98_COMPILE_EXTENSIONS} PARENT_SCOPE)
    set(CMAKE_CXX11_COMPILE_FEATURES ${CMAKE_CXX11_COMPILE_FEATURES} PARENT_SCOPE)
    set(CMAKE_CXX11_COMPILE_EXTENSIONS ${CMAKE_CXX11_COMPILE_EXTENSIONS} PARENT_SCOPE)

    message(STATUS "Detecting ${lang} compile features - done")
  endif()

endfunction()
