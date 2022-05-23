cmake_minimum_required(VERSION 3.17)

project(FindPkgConfig_IMPORTED_TARGET C)

set(shared_lib_prefix "")
set(shared_lib_suffix ".lib")
set(static_lib_prefix "lib")
set(static_lib_suffix ".a")

set(CMAKE_SHARED_LIBRARY_PREFIX ${shared_lib_prefix})
set(CMAKE_SHARED_LIBRARY_SUFFIX ${shared_lib_suffix})
set(CMAKE_STATIC_LIBRARY_PREFIX ${static_lib_prefix})
set(CMAKE_STATIC_LIBRARY_SUFFIX ${static_lib_suffix})

find_package(PkgConfig REQUIRED)

# to test multiple variations, we must pick unique prefix names (same-named targets are cached for re-use)
set(prefix_uniquifiers 0 1)
# whether to apply STATIC_TARGET argument
set(static_target_args "" STATIC_TARGET)
foreach (prefix_uniquifier static_target_arg IN ZIP_LISTS prefix_uniquifiers static_target_args)
  set(prefix "NCURSES${prefix_uniquifier}")
  message(STATUS "static_target_arg: ${static_target_arg}")
  pkg_check_modules(${prefix} IMPORTED_TARGET QUIET ${static_target_arg} ncurses)

  message(STATUS "source: ${CMAKE_CURRENT_SOURCE_DIR} bin ${CMAKE_CURRENT_BINARY_DIR}")

  if (${prefix}_FOUND)
    set(tgt PkgConfig::${prefix})
    message(STATUS "Verifying target \"${tgt}\"")
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
endforeach ()

# Setup for the remaining package tests below
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH)
set(fakePkgDir ${CMAKE_CURRENT_BINARY_DIR}/pc-fakepackage)
foreach(i 1 2)
  set(pname cmakeinternalfakepackage${i})
  file(WRITE ${fakePkgDir}/lib/${static_lib_prefix}${pname}${static_lib_suffix} "")
  file(WRITE ${fakePkgDir}/lib/${shared_lib_prefix}${pname}${shared_lib_suffix} "")
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

set(tgt PkgConfig::FakePackage1_dir)
if (TARGET ${tgt})
  message(FATAL_ERROR "imported target \"${tgt}\" is visible outside its directory")
endif()

set(tgt PkgConfig::FakePackage1_global)
if (NOT TARGET ${tgt})
  message(FATAL_ERROR "imported target \"${tgt}\" is not visible outside its directory")
endif()

# And now do the same for the NO_CMAKE_ENVIRONMENT_PATH - ENV{CMAKE_PREFIX_PATH}
# combination
unset(CMAKE_PREFIX_PATH)
unset(ENV{CMAKE_PREFIX_PATH})
set(ENV{CMAKE_PREFIX_PATH} ${fakePkgDir})


# to test multiple variations, we must pick unique prefix names (same-named targets are cached for re-use)
set(prefix_uniquifiers 0 1)
# whether to apply STATIC_TARGET argument
set(static_target_args "" STATIC_TARGET)
# whether target properties are populated from the unqualified (i.e. shared library) series of vars, or the STATIC_ series of vars
set(target_var_qualifiers "" STATIC_)
set(lib_types shared static)
foreach (prefix_uniquifier static_target_arg target_var_qualifier lib_type IN ZIP_LISTS prefix_uniquifiers static_target_args target_var_qualifiers lib_types)
  set(prefix "FakePackage2${prefix_uniquifier}")
  set(tgt "PkgConfig::${prefix}")
  pkg_check_modules(${prefix} REQUIRED QUIET IMPORTED_TARGET ${static_target_arg} cmakeinternalfakepackage2)

  message(STATUS "Verifying library path resolution for lib type \"${lib_type}\"")
  if (NOT TARGET ${tgt})
    message(FATAL_ERROR "No import target for fake package 2 with prefix path")
  endif()

  set(link_libraries_var ${prefix}_${target_var_qualifier}LINK_LIBRARIES)
  # check that 2 library entries exist
  list(LENGTH ${link_libraries_var} fp2_nlibs)
  if (NOT fp2_nlibs EQUAL 2)
    message(FATAL_ERROR "${link_libraries_var} has ${fp2_nlibs} entries but should have exactly 2")
  endif()

  set(lib_leafname ${${lib_type}_lib_prefix}cmakeinternalfakepackage2${${lib_type}_lib_suffix})
  message(STATUS "Expecting library leafname \"${lib_leafname}\"")
  # check that the full library path is also returned
  list(GET ${link_libraries_var} 0 fp2_lib0)
  if (NOT fp2_lib0 STREQUAL "${fakePkgDir}/lib/${lib_leafname}")
    message(FATAL_ERROR "${link_libraries_var} has bad content on first run: ${${link_libraries_var}}")
  endif()

  # check that the library that couldn't be found still shows up
  list(GET ${link_libraries_var} 1 fp2_lib1)
  if (NOT fp2_lib1 STREQUAL "cmakeinternalfakepackage2-doesnotexist")
    message(FATAL_ERROR "${link_libraries_var} has bad content on first run: ${${link_libraries_var}}")
  endif()

  # the information in *_LINK_LIBRARIES is not cached, so ensure is also is present on second run
  unset(${link_libraries_var})
  pkg_check_modules(${prefix} REQUIRED QUIET IMPORTED_TARGET ${static_target_arg} cmakeinternalfakepackage2)
  if (NOT ${link_libraries_var} STREQUAL "${fakePkgDir}/lib/${lib_leafname};cmakeinternalfakepackage2-doesnotexist")
    message(FATAL_ERROR "${link_libraries_var} has bad content on second run: ${${link_libraries_var}}")
  endif()
endforeach()

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

set(tgt PkgConfig::FakeLinkOptionsPackage)
message(STATUS "Verifying target \"${tgt}\"")
if (NOT TARGET ${tgt})
  message(FATAL_ERROR "No import target for fake link options package")
endif()
get_target_property(link_options ${tgt} INTERFACE_LINK_OPTIONS)
if (NOT link_options STREQUAL expected_link_options)
  message(FATAL_ERROR
    "Additional link options not present in INTERFACE_LINK_OPTIONS property\n"
    "expected: \"${expected_link_options}\", but got \"${link_options}\""
  )
endif()

get_target_property(inc_dirs ${tgt} INTERFACE_INCLUDE_DIRECTORIES)
set(expected_inc_dirs "/special" "/other" "/more")

if (NOT inc_dirs STREQUAL expected_inc_dirs)
  message(FATAL_ERROR
    "Additional include directories not correctly present in INTERFACE_INCLUDE_DIRECTORIES property\n"
    "expected: \"${expected_inc_dirs}\", got \"${inc_dirs}\""
  )
endif ()

get_target_property(c_opts ${tgt} INTERFACE_COMPILE_OPTIONS)
set(expected_c_opts "-DA-isystem/foo") # this is an invalid option, but a good testcase
if (NOT c_opts STREQUAL expected_c_opts)
    message(FATAL_ERROR
      "Additional compile options not present in INTERFACE_COMPILE_OPTIONS property\n"
      "expected: \"${expected_c_opts}\", got \"${c_opts}\""
    )
endif ()
