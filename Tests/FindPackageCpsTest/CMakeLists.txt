cmake_minimum_required(VERSION 3.31)
project(FindPackageCpsTest)

set(CMAKE_EXPERIMENTAL_FIND_CPS_PACKAGES "e82e467b-f997-4464-8ace-b00808fff261")

# Protect tests from running inside the default install prefix.
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/NotDefaultPrefix")

# Use the test directory to Look for packages.
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Disable built-in search paths.
set(CMAKE_FIND_USE_PACKAGE_ROOT_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH OFF)
set(CMAKE_FIND_USE_INSTALL_PREFIX OFF)

# Enable framework searching.
set(CMAKE_FIND_FRAMEWORK FIRST)

add_executable(FindPackageCpsTest FindPackageTest.cxx)

###############################################################################

function(expect PACKAGE VAR OP VALUE WHAT)
  if(NOT ${PACKAGE}_${VAR} ${OP} ${VALUE})
    message(SEND_ERROR "${PACKAGE} wrong ${WHAT} ${${PACKAGE}_${VAR}} !")
  endif()
endfunction()

function(test_version PACKAGE LITERAL COUNT MAJOR MINOR PATCH TWEAK)
  if(NOT ${PACKAGE}_FOUND)
    message(SEND_ERROR "${PACKAGE} not found !")
  else()
    expect(${PACKAGE} VERSION STREQUAL "${LITERAL}" "version")
    expect(${PACKAGE} VERSION_COUNT EQUAL ${COUNT} "version count")
    expect(${PACKAGE} VERSION_MAJOR EQUAL ${MAJOR} "major version")
    expect(${PACKAGE} VERSION_MINOR EQUAL ${MINOR} "minor version")
    expect(${PACKAGE} VERSION_PATCH EQUAL ${PATCH} "patch version")
    expect(${PACKAGE} VERSION_TWEAK EQUAL ${TWEAK} "tweak version")
  endif()
endfunction()

function(test_unparsed_version PACKAGE VERSION)
  find_package(${PACKAGE} CONFIG)
  test_version(${PACKAGE} "${VERSION}" 0 0 0 0 0)
endfunction()

###############################################################################
# Test a basic package search.
# It should NOT find the package's CMake package file.

find_package(Sample CONFIG)
test_version(Sample "2.10.11" 3 2 10 11 0)

###############################################################################
# Test some more complicated version parsing.

find_package(LongVersion CONFIG)
test_version(LongVersion "1.1.2.3.5.8+fibonacci" 6 1 1 2 3)

find_package(EmptyMarker CONFIG)
test_version(EmptyMarker "1.1+" 2 1 1 0 0)

test_unparsed_version(BadVersion1 "1..1")
test_unparsed_version(BadVersion2 "1.1a.0")
test_unparsed_version(BadVersion3 "1.1.")
test_unparsed_version(BadVersion4 "+42")
test_unparsed_version(CustomVersion "VII")

###############################################################################
# Test finding a package whose CPS file is in the package prefix root.
set(RootTest_DIR "${CMAKE_CURRENT_SOURCE_DIR}/RootTest")
find_package(RootTest)
if(NOT RootTest_FOUND)
  message(SEND_ERROR "RootTest not found !")
endif()

###############################################################################
# Test glob sorting.

set(SortLib_DIR "" CACHE FILEPATH "Wipe out find results for testing." FORCE)
set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION ASC)
find_package(SortLib CONFIG)
if(NOT "${SortLib_VERSION}" STREQUAL "3.1.1")
  message(SEND_ERROR "FIND_PACKAGE_SORT_ORDER Name Asc! ${SortLib_VERSION}")
endif()
unset(SortLib_VERSION)

set(SortLib_DIR "" CACHE FILEPATH "Wipe out find results for testing." FORCE)
set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)
find_package(SortLib CONFIG)
if(NOT "${SortLib_VERSION}" STREQUAL "3.10.1")
  message(SEND_ERROR "FIND_PACKAGE_SORT_ORDER Natural! Dec ${SortLib_VERSION}")
endif()
set(SortLib_DIR "" CACHE FILEPATH "Wipe out find results for testing." FORCE)
unset(SortLib_VERSION)

set(SortLib_DIR "" CACHE FILEPATH "Wipe out find results for testing." FORCE)
FIND_PACKAGE(SortLib 4.0 CONFIG)
IF (NOT "${SortLib_VERSION}" STREQUAL "4.0.0")
  message(SEND_ERROR "FIND_PACKAGE_SORT_ORDER gave up too soon! ${SortLib_VERSION}")
endif()
unset(SortLib_VERSION)

set(SortFramework_DIR "" CACHE FILEPATH "Wipe out find results for testing." FORCE)
set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION ASC)
find_package(SortFramework CONFIG)
if(NOT "${SortFramework_VERSION}" STREQUAL "3.1.1")
  message(SEND_ERROR "FIND_PACKAGE_SORT_ORDER Framework Name Asc! ${SortFramework_VERSION}")
endif()
set(SortLib_DIR "" CACHE FILEPATH "Wipe out find results for testing." FORCE)
unset(SortFramework_VERSION)

set(SortFramework_DIR "" CACHE FILEPATH "Wipe out find results for testing." FORCE)
set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)
find_package(SortFramework CONFIG)
if(NOT "${SortFramework_VERSION}" STREQUAL "3.10.1")
  message(SEND_ERROR "FIND_PACKAGE_SORT_ORDER Framework Natural! Dec ${SortFramework_VERSION}")
endif()
set(SortLib_DIR "" CACHE FILEPATH "Wipe out find results for testing." FORCE)
unset(SortFramework_VERSION)

unset(CMAKE_FIND_PACKAGE_SORT_ORDER)
unset(CMAKE_FIND_PACKAGE_SORT_DIRECTION)

###############################################################################
# Find a package that actually has some content.

find_package(Foo CONFIG)
if(NOT Foo_FOUND)
  message(SEND_ERROR "Foo not found !")
elseif(NOT TARGET Foo::PrefixTest)
  message(SEND_ERROR "Foo::PrefixTest missing !")
elseif(NOT TARGET Foo::RelativeTest)
  message(SEND_ERROR "Foo::RelativeTest missing !")
elseif(NOT TARGET Foo::Empty)
  message(SEND_ERROR "Foo::Empty missing !")
else()
  get_property(pt_includes
    TARGET Foo::PrefixTest PROPERTY INTERFACE_INCLUDE_DIRECTORIES_DEFAULT)
  if(NOT "${CMAKE_CURRENT_SOURCE_DIR}/include" PATH_EQUAL "${pt_includes}")
    message(SEND_ERROR "Foo::PrefixTest has wrong includes '${pt_includes}' !")
  endif()
  set(pt_includes)

  get_property(rt_includes
    TARGET Foo::RelativeTest PROPERTY INTERFACE_INCLUDE_DIRECTORIES_DEFAULT)
  if(NOT "${CMAKE_CURRENT_SOURCE_DIR}/cps/../include" PATH_EQUAL "${rt_includes}")
    message(SEND_ERROR "Foo::RelativeTest has wrong includes '${rt_includes}' !")
  endif()
  set(rt_includes)
endif()

###############################################################################
# Test importing of compile definitions.

find_package(defs CONFIG REQUIRED)

if(CMAKE_GENERATOR STREQUAL "Xcode" OR CMAKE_GENERATOR MATCHES "Visual Studio")
  # VS/Xcode generators cannot use different definitions within the same
  # target, so we must split the test into one target per language.
  add_library(defs-test-c STATIC defs-test-c.c)
  add_library(defs-test-cxx STATIC defs-test-cxx.cxx)
  target_link_libraries(defs-test-c defs::defs)
  target_link_libraries(defs-test-cxx defs::defs)
else()
  add_library(defs-test STATIC defs-test-c.c defs-test-cxx.cxx)
  target_link_libraries(defs-test defs::defs)
endif()

###############################################################################
# Test importing of (language-specific) include paths.

include(CheckIncludeFile)
include(CheckIncludeFileCXX)

find_package(includes CONFIG REQUIRED)

set(CMAKE_REQUIRED_LIBRARIES includes::default)
check_include_file(cmincludetest/global.h C_GLOBAL_H)
check_include_file(cmincludetest/cxxonly.h C_CXXONLY_H)
check_include_file_cxx(cmincludetest/cxxonly.h CXX_CXXONLY_H)
set(CMAKE_REQUIRED_LIBRARIES)

if(NOT C_GLOBAL_H)
  message(SEND_ERROR "cmincludetest/global.h not found !")
endif()
if(NOT CXX_CXXONLY_H)
  message(SEND_ERROR "cmincludetest/cxxonly.h not found in C++ mode !")
endif()
if(C_CXXONLY_H)
  message(SEND_ERROR "cmincludetest/cxxonly.h unexpectedly found in C mode ?!")
endif()

###############################################################################
# Find a package that has dependencies.

find_package(Bar)
if(NOT Bar_FOUND)
  message(SEND_ERROR "Bar not found !")
elseif(NOT Dep1_FOUND)
  message(SEND_ERROR "Bar's Dep1 not found !")
elseif(NOT Dep2_FOUND)
  message(SEND_ERROR "Bar's Dep2 not found !")
elseif(NOT Dep3_FOUND)
  message(SEND_ERROR "Bar's Dep3 not found !")
elseif(NOT TARGET Dep1::Target)
  message(SEND_ERROR "Dep1::Target missing !")
elseif(NOT TARGET Dep2::Target)
  message(SEND_ERROR "Dep2::Target missing !")
elseif(NOT TARGET Dep3::Target)
  message(SEND_ERROR "Dep3::Target missing !")
elseif(NOT TARGET Bar::Target1)
  message(SEND_ERROR "Bar::Target1 missing !")
elseif(NOT TARGET Bar::Target2)
  message(SEND_ERROR "Bar::Target2 missing !")
endif()

###############################################################################
# Test requesting components from a package.

find_package(ComponentTest
  COMPONENTS Target1 Target2
  OPTIONAL_COMPONENTS Target4 Target6)
if(NOT ComponentTest_FOUND)
  message(SEND_ERROR "ComponentTest not found !")
elseif(NOT TARGET ComponentTest::Target1)
  message(SEND_ERROR "ComponentTest::Target1 missing !")
elseif(NOT TARGET ComponentTest::Target2)
  message(SEND_ERROR "ComponentTest::Target2 missing !")
elseif(NOT TARGET ComponentTest::Target3)
  message(SEND_ERROR "ComponentTest::Target3 missing !")
elseif(NOT TARGET ComponentTest::Target4)
  message(SEND_ERROR "ComponentTest::Target4 missing !")
elseif(NOT TARGET ComponentTest::Target5)
  message(SEND_ERROR "ComponentTest::Target5 missing !")
elseif(TARGET ComponentTest::Target6)
  message(SEND_ERROR "ComponentTest::Target6 exists ?!")
elseif(TARGET ComponentTest::Target7)
  message(SEND_ERROR "ComponentTest::Target7 exists ?!")
elseif(TARGET ComponentTest::Target8)
  message(SEND_ERROR "ComponentTest::Target8 exists ?!")
endif()

###############################################################################
# Test requesting components from a dependency.

find_package(TransitiveTest)
if(NOT TransitiveTest_FOUND)
  message(SEND_ERROR "TransitiveTest not found !")
elseif(NOT TransitiveDep_FOUND)
  message(SEND_ERROR "TransitiveTest's TransitiveDep not found !")
elseif(NOT TARGET TransitiveDep::Target1)
  message(SEND_ERROR "TransitiveDep::Target1 missing !")
elseif(NOT TARGET TransitiveDep::Target2)
  message(SEND_ERROR "TransitiveDep::Target2 missing !")
elseif(NOT TARGET TransitiveDep::Target3)
  message(SEND_ERROR "TransitiveDep::Target3 missing !")
elseif(TARGET TransitiveDep::Target4)
  message(SEND_ERROR "TransitiveDep::Target4 exists ?!")
elseif(TARGET TransitiveDep::Target5)
  message(SEND_ERROR "TransitiveDep::Target5 exists ?!")
endif()

###############################################################################
# Test default configurations.

find_package(DefaultConfigurationsTest)
if(NOT DefaultConfigurationsTest_FOUND)
  message(SEND_ERROR "DefaultConfigurationsTest not found !")
elseif(NOT TARGET DefaultConfigurationsTest::Target)
  message(SEND_ERROR "DefaultConfigurationsTest::Target missing !")
else()
  get_property(dct_configs
    TARGET DefaultConfigurationsTest::Target PROPERTY IMPORTED_CONFIGURATIONS)
  if(NOT "${dct_configs}" STREQUAL "DEFAULT;TEST")
    message(SEND_ERROR "DefaultConfigurationsTest::Target has wrong configurations '${dct_configs}' !")
  endif()
  set(dct_configs)
endif()
