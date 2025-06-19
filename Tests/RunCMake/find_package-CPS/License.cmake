cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

find_package(LicenseTest REQUIRED)

function(expect_license COMPONENT EXPECTED)
  set(target LicenseTest::${COMPONENT})
  if(TARGET ${target})
    get_target_property(license ${target} "SPDX_LICENSE")
    if (NOT "${license}" STREQUAL "${EXPECTED}")
      message(SEND_ERROR
        "Target ${target} has wrong license '${license}'"
        " (expected '${EXPECTED}') !")
    endif()
  else()
    message(SEND_ERROR "Expected target ${target} was not found !")
  endif()
endfunction()

expect_license(SpecifiedOnTarget "Apache-2.0")
expect_license(InheritFromRoot "BSD-3-Clause")
expect_license(InheritFromAppendix "Apache-2.0")
expect_license(DisableInheritance "license-NOTFOUND")
