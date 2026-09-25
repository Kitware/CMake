cmake_minimum_required(VERSION 3.31)
project(FindPackageCpsTest)

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

function(expect_property TARGET PROPERTY DESCRIPTION)
  get_target_property(actual ${TARGET} ${PROPERTY})
  if(NOT "${actual}" STREQUAL "${ARGN}")
    message(SEND_ERROR "${TARGET} has wrong ${DESCRIPTION} '${actual}' !")
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

function(expect_global TARGET VALUE)
  get_target_property(global "${TARGET}" IMPORTED_GLOBAL)
  if((VALUE AND NOT global) OR (NOT VALUE AND global))
    message(SEND_ERROR "${TARGET} has wrong IMPORTED_GLOBAL '${global}' !")
  endif()
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

foreach(CMAKE_FIND_PACKAGE_SORT_DIRECTION IN ITEMS "" ASC Bogus)
  set(SortLib_DIR "" CACHE FILEPATH "Wipe out find results for testing." FORCE)
  set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
  find_package(SortLib CONFIG)
  if(NOT "${SortLib_VERSION}" STREQUAL "3.1.1")
    message(SEND_ERROR "FIND_PACKAGE_SORT_ORDER Name Asc! ${SortLib_VERSION}")
  endif()
  unset(SortLib_VERSION)
endforeach()


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


set(SortLib_DIR "" CACHE FILEPATH "Wipe out find results for testing." FORCE)
# Expected to default to 'NATURAL' and 'DEC'
unset(CMAKE_FIND_PACKAGE_SORT_ORDER)
unset(CMAKE_FIND_PACKAGE_SORT_DIRECTION)
FIND_PACKAGE(SortLib CONFIG)
IF (NOT "${SortLib_VERSION}" STREQUAL "3.10.1")
  message(SEND_ERROR "FIND_PACKAGE_SORT_ORDER Default! ${SortLib_VERSION}")
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
elseif(NOT TARGET Foo::ExecutableTest)
  message(SEND_ERROR "Foo::ExecutableTest missing !")
elseif(NOT TARGET Foo::Empty)
  message(SEND_ERROR "Foo::Empty missing !")
else()
  expect_property(Foo::PrefixTest TYPE "type" "INTERFACE_LIBRARY")
  expect_property(
    Foo::PrefixTest INTERFACE_INCLUDE_DIRECTORIES "includes"
    "$<$<CONFIG:default>:${CMAKE_CURRENT_SOURCE_DIR}/include>")

  expect_property(Foo::RelativeTest TYPE "type" "INTERFACE_LIBRARY")
  expect_property(
    Foo::RelativeTest INTERFACE_INCLUDE_DIRECTORIES "includes"
    "$<$<CONFIG:default>:${CMAKE_CURRENT_SOURCE_DIR}/cps/../include>")

  expect_property(Foo::ExecutableTest TYPE "type" "EXECUTABLE")
  expect_property(Foo::ExecutableTest IMPORTED_LOCATION_DEFAULT "location"
    "${CMAKE_CURRENT_SOURCE_DIR}/foo")

  expect_global(Foo::Empty FALSE)
endif()

###############################################################################
# Test importing into global scope.

find_package(Global CONFIG GLOBAL)
if(NOT Global_FOUND)
  message(SEND_ERROR "Global not found !")
elseif(NOT TARGET Global::Target)
  message(SEND_ERROR "Global::Target missing !")
else()
  expect_global(Global::Target TRUE)
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
# Test importing and mangling of requirements (i.e. link libraries).

find_package(RequiresTest CONFIG REQUIRED)

add_executable(requires-test requires-test.cxx)
target_link_libraries(requires-test RequiresTest::Indirect)

add_executable(requires-test-compile_only requires-test.cxx)
target_link_libraries(requires-test-compile_only RequiresTest::CompileOnly)

expect_property(
  RequiresTest::Other INTERFACE_LINK_LIBRARIES
  "link libraries" "every-config;$<$<CONFIG:test>:test-config>")

expect_property(
  RequiresTest::Other IMPORTED_LINK_DEPENDENT_LIBRARIES
  "dyld requirements (any config)" "RequiresTest::Private1")

expect_property(
  RequiresTest::Other IMPORTED_LINK_DEPENDENT_LIBRARIES_TEST
  "dyld requirements ('TEST' config)" "RequiresTest::Private2")

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
# Find some packages with malformatted dependencies.

find_package(Bad1)
if(NOT Bad1_FOUND)
  message(SEND_ERROR "Bad1 not found !")
endif()

find_package(Bad2)
if(NOT Bad2_FOUND)
  message(SEND_ERROR "Bad2 not found !")
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
# Test finding a package more than once.

find_package(Repeat REQUIRED OPTIONAL_COMPONENTS DoesNotExist)
if(NOT TARGET Repeat::Base)
  message(SEND_ERROR "Repeat::Base missing !")
elseif(TARGET Repeat::Extra)
  message(SEND_ERROR "Repeat::Extra exists ?!")
endif()

find_package(Repeat REQUIRED COMPONENTS Extra)
if(NOT TARGET Repeat::Extra)
  message(SEND_ERROR "Repeat::Extra missing !")
endif()

###############################################################################
# Test default configurations.

find_package(DefaultConfigurationsTest)
if(NOT DefaultConfigurationsTest_FOUND)
  message(SEND_ERROR "DefaultConfigurationsTest not found !")
elseif(NOT TARGET DefaultConfigurationsTest::Target1)
  message(SEND_ERROR "DefaultConfigurationsTest::Target1 missing !")
elseif(NOT TARGET DefaultConfigurationsTest::Target2)
  message(SEND_ERROR "DefaultConfigurationsTest::Target2 missing !")
else()
  expect_property(
    DefaultConfigurationsTest::Target1 IMPORTED_CONFIGURATIONS
    "configurations" "DEFAULT;TEST")
  expect_property(
    DefaultConfigurationsTest::Target2 IMPORTED_CONFIGURATIONS
    "configurations" "TEST")
endif()

###############################################################################
# Test unnamed file sets.

find_package(FileSets-Anonymous)
if(NOT FileSets-Anonymous_FOUND)
  message(SEND_ERROR "FileSets-Anonymous not found !")
elseif(NOT TARGET FileSets-Anonymous::Target)
  message(SEND_ERROR "FileSets-Anonymous::Target missing !")
else()
  expect_property(
    FileSets-Anonymous::Target INTERFACE_HEADER_SETS
    "include set names" "HEADERS")
  expect_property(
    FileSets-Anonymous::Target HEADER_DIRS
    "include set roots"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/include")
  expect_property(
    FileSets-Anonymous::Target HEADER_DIRS_HEADERS
    "include set 'HEADERS' roots"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/include")
  expect_property(
    FileSets-Anonymous::Target HEADER_SET
    "include set files"
    "${CMAKE_SOURCE_DIR}/include/header1.h"
    "${CMAKE_SOURCE_DIR}/include/header2.h"
    "${CMAKE_SOURCE_DIR}/include/header3.h")
  expect_property(
    FileSets-Anonymous::Target HEADER_SET_HEADERS
    "include set 'HEADERS' files"
    "${CMAKE_SOURCE_DIR}/include/header1.h"
    "${CMAKE_SOURCE_DIR}/include/header2.h"
    "${CMAKE_SOURCE_DIR}/include/header3.h")
endif()

###############################################################################
# Test named file sets.

find_package(FileSets-Named)
if(NOT FileSets-Named_FOUND)
  message(SEND_ERROR "FileSets-Named not found !")
elseif(NOT TARGET FileSets-Named::Target)
  message(SEND_ERROR "FileSets-Named::Target missing !")
else()
  expect_property(
    FileSets-Named::Target INTERFACE_HEADER_SETS
    "include set names" "foo;bar")
  expect_property(
    FileSets-Named::Target HEADER_DIRS_foo
    "include set roots"
    "${CMAKE_SOURCE_DIR}/include/a"
    "${CMAKE_SOURCE_DIR}/include/b")
  expect_property(
    FileSets-Named::Target HEADER_DIRS_foo
    "include set 'foo' roots"
    "${CMAKE_SOURCE_DIR}/include/a"
    "${CMAKE_SOURCE_DIR}/include/b")
  expect_property(
    FileSets-Named::Target HEADER_DIRS_bar
    "include set 'bar' roots"
    "${CMAKE_SOURCE_DIR}/include")
  expect_property(
    FileSets-Named::Target HEADER_SET_foo
    "include set 'foo' files"
    "${CMAKE_SOURCE_DIR}/include/a/foo1.h"
    "${CMAKE_SOURCE_DIR}/include/a/foo2.h"
    "${CMAKE_SOURCE_DIR}/include/b/foo3.h")
  expect_property(
    FileSets-Named::Target HEADER_SET_bar
    "include set 'bar' files"
    "${CMAKE_SOURCE_DIR}/include/bar.h")
endif()

###############################################################################
# Test CPS + pkg-config interoperability.

set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/pc")
find_package(pc-link-test)
if(NOT pc-link-test_FOUND)
  message(SEND_ERROR "pc-link-test not found !")
elseif(NOT TARGET pc-link-test::Parent)
  message(SEND_ERROR "pc-link-test::Parent missing !")
elseif(NOT TARGET pc-link-test::InheritsRequirement)
  message(SEND_ERROR "pc-link-test::InheritsRequirement missing !")
elseif(NOT TARGET pc-link-test::ShadowsRequirement)
  message(SEND_ERROR "pc-link-test::ShadowsRequirement missing !")
elseif(NOT TARGET pc-link-test::UnknownDomain)
  message(SEND_ERROR "pc-link-test::UnknownDomain missing !")
endif()

expect_property(pc-link-test::Parent INTERFACE_LINK_LIBRARIES
  "link libraries (from pkg-config dependency)"
  "@foreign_pkgcfg::cps-pkg-test"
  "$<$<CONFIG:debug>:$<COMPILE_ONLY:@foreign_pkgcfg::cps-pkg-test>>"
  "$<$<CONFIG:debug>:$<LINK_ONLY:@foreign_pkgcfg::cps-pkg-test>>")
expect_property(pc-link-test::Parent IMPORTED_LINK_DEPENDENT_LIBRARIES_DEBUG
  "dyld requirements (from pkg-config dependency)"
  "@foreign_pkgcfg::cps-pkg-test")

# Appendices inherit base requirement domains and keep additional requirements
# local. Configuration supplements inherit both levels of requirement domains.
# Their package-level requires blocks are ignored, including attempted
# overrides of the inherited pkg-config domains.
expect_property(pc-link-test::InheritsRequirement INTERFACE_LINK_LIBRARIES
  "link libraries (from inherited and local dependencies)"
  "@foreign_pkgcfg::cps-pkg-test"
  "@foreign_pkgcfg::cps-pkg-appendix"
  "$<$<CONFIG:debug>:@foreign_pkgcfg::cps-pkg-test>"
  "$<$<CONFIG:debug>:@foreign_pkgcfg::cps-pkg-appendix>")
expect_property(pc-link-test::ShadowsRequirement INTERFACE_LINK_LIBRARIES
  "link libraries (from shadowing dependency)"
  "cps-pkg-test::cps-pkg-test"
  "$<COMPILE_ONLY:cps-pkg-test::cps-pkg-test>"
  "$<LINK_ONLY:cps-pkg-test::cps-pkg-test>")
expect_property(pc-link-test::ShadowsRequirement IMPORTED_LINK_DEPENDENT_LIBRARIES
  "dyld requirements (from shadowing dependency)"
  "cps-pkg-test::cps-pkg-test")
expect_property(pc-link-test::UnknownDomain INTERFACE_LINK_LIBRARIES
  "link libraries (from unknown domain dependency)"
  "cps-pkg-test::cps-pkg-test")
