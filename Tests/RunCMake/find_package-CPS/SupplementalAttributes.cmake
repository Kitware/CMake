cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

find_package(SupplementalAttributesTest REQUIRED COMPONENTS Sample)

get_target_property(license SupplementalAttributesTest::Sample "SPDX_LICENSE")
if (NOT "${license}" STREQUAL "BSD-3-Clause")
  message(SEND_ERROR "SupplementalAttributesTest wrong license ${license} !")
endif()
