cmake_minimum_required(VERSION 3.14)
project(TestFindBoostPython CXX)
include(CTest)

find_package(Boost OPTIONAL_COMPONENTS python27 python34 python35 python36 python37)

set(FAILTEST TRUE)
foreach (v IN ITEMS 27 34 35 36 37)
  if (Boost_PYTHON${v}_FOUND)
    set(FAILTEST FALSE)
    break()
  endif ()
endforeach ()

if (FAILTEST)
  message(FATAL_ERROR "No Boost Python module found")
endif ()
