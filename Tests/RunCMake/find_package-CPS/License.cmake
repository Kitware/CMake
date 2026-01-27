cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

find_package(LicenseTest REQUIRED)

function(expect_license COMPONENT EXPECTED)
  if(TARGET ${COMPONENT})
    get_target_property(license ${COMPONENT} "SPDX_LICENSE")
    if (NOT "${license}" STREQUAL "${EXPECTED}")
      message(SEND_ERROR
        "Target ${COMPONENT} has wrong license '${license}'"
        " (expected '${EXPECTED}') !")
    endif()
  else()
    message(SEND_ERROR "Expected target ${COMPONENT} was not found !")
  endif()
endfunction()

expect_license(LicenseTest::SpecifiedOnTarget "Apache-2.0")
expect_license(LicenseTest::InheritFromRoot "BSD-3-Clause")
expect_license(LicenseTest::InheritFromAppendix "Apache-2.0")
expect_license(LicenseTest::DisableInheritance "license-NOTFOUND")

find_package(PackageLicenseTest REQUIRED)

expect_license(PackageLicenseTest::SpecifiedOnTarget "Apache-2.0")
expect_license(PackageLicenseTest::InheritFromRoot "BSD-3-Clause")
