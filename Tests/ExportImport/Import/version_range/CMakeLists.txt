
cmake_minimum_required(VERSION 3.18)

find_package(testLibRequired 2.0...3.0)

if (NOT testLibRequired_FOUND)
  message(SEND_ERROR "version_range: fail to find package testLibRequired(2.5) with range 2.0...3.0")
endif()


find_package(testLibRequired 2.0...<2.5)

if (testLibRequired_FOUND)
  message(SEND_ERROR "version_range: package testLibRequired(2.5) unexpectedly found with range 2.0...<2.5")
endif()
