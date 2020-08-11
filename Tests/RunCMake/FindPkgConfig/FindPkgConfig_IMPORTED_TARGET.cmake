cmake_minimum_required(VERSION 3.12)

project(FindPkgConfig_IMPORTED_TARGET C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES IMPORTED_TARGET QUIET ncurses)

message(STATUS "source: ${CMAKE_CURRENT_SOURCE_DIR} bin ${CMAKE_CURRENT_BINARY_DIR}")

if (NCURSES_FOUND)
  set(tgt PkgConfig::NCURSES)
  if (NOT TARGET ${tgt})
    message(FATAL_ERROR "FindPkgConfig found ncurses, but did not create an imported target for it")
  endif ()
  set(prop_found FALSE)
  foreach (prop IN ITEMS INTERFACE_INCLUDE_DIRECTORIES INTERFACE_LINK_LIBRARIES INTERFACE_COMPILE_OPTIONS)
    get_target_property(value ${tgt} ${prop})
    if (value)
      message(STATUS "Found property ${prop} on target: ${value}")
      set(prop_found TRUE)
    endif ()
  endforeach ()
  if (NOT prop_found)
    message(FATAL_ERROR "target ${tgt} found, but it has no properties")
  endif ()
else ()
  message(STATUS "skipping test; ncurses not found")
endif ()


# Setup for the remaining package tests below
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH)
set(fakePkgDir ${CMAKE_CURRENT_BINARY_DIR}/pc-fakepackage)
foreach(i 1 2)
  set(pname cmakeinternalfakepackage${i})
  file(WRITE ${fakePkgDir}/lib/lib${pname}.a "")
  file(WRITE ${fakePkgDir}/lib/${pname}.lib  "")
  file(WRITE ${fakePkgDir}/lib/pkgconfig/${pname}.pc
"Name: CMakeInternalFakePackage${i}
Description: Dummy package (${i}) for FindPkgConfig IMPORTED_TARGET test
Version: 1.2.3
Libs: -l${pname} -l${pname}-doesnotexist
")
endforeach()

# Always find the .pc file in the calls further below so that we can test that
# the import target find_library() calls handle the NO...PATH options correctly
set(ENV{PKG_CONFIG_PATH} ${fakePkgDir}/lib/pkgconfig)

# find targets in subdir and check their visibility
add_subdirectory(target_subdir)
if (TARGET PkgConfig::FakePackage1_dir)
  message(FATAL_ERROR "imported target PkgConfig::FakePackage1_dir is visible outside it's directory")
endif()

if (NOT TARGET PkgConfig::FakePackage1_global)
  message(FATAL_ERROR "imported target PkgConfig::FakePackage1_global is not visible outside it's directory")
endif()

# And now do the same for the NO_CMAKE_ENVIRONMENT_PATH - ENV{CMAKE_PREFIX_PATH}
# combination
unset(CMAKE_PREFIX_PATH)
unset(ENV{CMAKE_PREFIX_PATH})
set(ENV{CMAKE_PREFIX_PATH} ${fakePkgDir})

pkg_check_modules(FakePackage2 REQUIRED QUIET IMPORTED_TARGET cmakeinternalfakepackage2)
if (NOT TARGET PkgConfig::FakePackage2)
  message(FATAL_ERROR "No import target for fake package 2 with prefix path")
endif()

# check that 2 library entries exist
list(LENGTH FakePackage2_LINK_LIBRARIES fp2_nlibs)
if (NOT fp2_nlibs EQUAL 2)
  message(FATAL_ERROR "FakePackage2_LINK_LIBRARIES has ${fp2_nlibs} entries but should have exactly 2")
endif()

# check that the full library path is also returned
list(GET FakePackage2_LINK_LIBRARIES 0 fp2_lib0)
if (NOT fp2_lib0 STREQUAL "${fakePkgDir}/lib/libcmakeinternalfakepackage2.a")
  message(FATAL_ERROR "FakePackage2_LINK_LIBRARIES has bad content on first run: ${FakePackage2_LINK_LIBRARIES}")
endif()

# check that the library that couldn't be found still shows up
list(GET FakePackage2_LINK_LIBRARIES 1 fp2_lib1)
if (NOT fp2_lib1 STREQUAL "cmakeinternalfakepackage2-doesnotexist")
  message(FATAL_ERROR "FakePackage2_LINK_LIBRARIES has bad content on first run: ${FakePackage2_LINK_LIBRARIES}")
endif()

# the information in *_LINK_LIBRARIES is not cached, so ensure is also is present on second run
unset(FakePackage2_LINK_LIBRARIES)
pkg_check_modules(FakePackage2 REQUIRED QUIET IMPORTED_TARGET cmakeinternalfakepackage2)
if (NOT FakePackage2_LINK_LIBRARIES STREQUAL "${fakePkgDir}/lib/libcmakeinternalfakepackage2.a;cmakeinternalfakepackage2-doesnotexist")
  message(FATAL_ERROR "FakePackage2_LINK_LIBRARIES has bad content on second run: ${FakePackage2_LINK_LIBRARIES}")
endif()

set(pname fakelinkoptionspackage)
file(WRITE ${fakePkgDir}/lib/pkgconfig/${pname}.pc
"Name: FakeLinkOptionsPackage
Description: Dummy package for FindPkgConfig IMPORTED_TARGET INTERFACE_LINK_OPTIONS test
Version: 1.2.3
Libs: -e dummy_main
Cflags: -I/special -isystem /other -isystem/more -DA-isystem/foo
")

set(expected_link_options -e dummy_main)
pkg_check_modules(FakeLinkOptionsPackage REQUIRED QUIET IMPORTED_TARGET fakelinkoptionspackage)
if (NOT TARGET PkgConfig::FakeLinkOptionsPackage)
  message(FATAL_ERROR "No import target for fake link options package")
endif()
get_target_property(link_options PkgConfig::FakeLinkOptionsPackage INTERFACE_LINK_OPTIONS)
if (NOT link_options STREQUAL expected_link_options)
  message(FATAL_ERROR
    "Additional link options not present in INTERFACE_LINK_OPTIONS property\n"
    "expected: \"${expected_link_options}\", but got \"${link_options}\""
  )
endif()

get_target_property(inc_dirs PkgConfig::FakeLinkOptionsPackage INTERFACE_INCLUDE_DIRECTORIES)
set(expected_inc_dirs "/special" "/other" "/more")

if (NOT inc_dirs STREQUAL expected_inc_dirs)
  message(FATAL_ERROR
    "Additional include directories not correctly present in INTERFACE_INCLUDE_DIRECTORIES property\n"
    "expected: \"${expected_inc_dirs}\", got \"${inc_dirs}\""
  )
endif ()

get_target_property(c_opts PkgConfig::FakeLinkOptionsPackage INTERFACE_COMPILE_OPTIONS)
set(expected_c_opts "-DA-isystem/foo") # this is an invalid option, but a good testcase
if (NOT c_opts STREQUAL expected_c_opts)
    message(FATAL_ERROR
      "Additional compile options not present in INTERFACE_COMPILE_OPTIONS property\n"
      "expected: \"${expected_c_opts}\", got \"${c_opts}\""
    )
endif ()
