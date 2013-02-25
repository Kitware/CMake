cmake_minimum_required(VERSION 2.6)
project(testRebuild)

function(test_for_xcode4 result_var)
  set(${result_var} 0 PARENT_SCOPE)
  if(APPLE)
    execute_process(COMMAND xcodebuild -version
      OUTPUT_VARIABLE ov RESULT_VARIABLE rv
      )
    if("${rv}" STREQUAL "0")
      if(ov MATCHES "^Xcode 4.[0-9].*$")
        set(${result_var} 1 PARENT_SCOPE)
      endif()
    endif()
  endif()
endfunction()

if(APPLE)
  # only use multi-arch if the sysroot exists on this machine
  # Ninja needs -M which could not be used with multiple -arch flags
  if(EXISTS "${CMAKE_OSX_SYSROOT}" AND NOT "${CMAKE_GENERATOR}" MATCHES "Ninja")
    set(CMAKE_OSX_ARCHITECTURES "ppc;i386")
    test_for_xcode4(is_xcode4)
    if(is_xcode4)
      # Xcode 4, use modern architectures as defaults
      # Arch 'ppc' no longer works: tools no longer available starting with Xcode 4
      set(CMAKE_OSX_ARCHITECTURES i386 x86_64)
    endif()
  endif()
endif()

add_library(foo STATIC ${testRebuild_BINARY_DIR}/foo.cxx)
set_target_properties(foo PROPERTIES OUTPUT_NAME "foolib")
# Add a generated header that regenerates when the generator is
# rebuilt.
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/regen.h
  COMMAND generator ${CMAKE_CURRENT_BINARY_DIR}/regen.h regen
  DEPENDS generator # adds file-level dependency to re-run rule
  )

# Add a generated header that does NOT regenerate when the generator
# is rebuilt.
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/noregen.h
  COMMAND generator ${CMAKE_CURRENT_BINARY_DIR}/noregen.h noregen
  )

# Test that the generator rebuilds when the static library source file
# changes.  This should cause regen.h to be recreated also.
add_executable(generator generator.cxx)
target_link_libraries(generator foo)
set_target_properties(generator PROPERTIES OUTPUT_NAME "gen")

# Build an executable to drive the build and rebuild.
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(bar bar.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/regen.h
  ${CMAKE_CURRENT_BINARY_DIR}/noregen.h
  )

#-----------------------------------------------------------------------------
if("${CMAKE_GENERATOR}" MATCHES "Make")
  # Test the IMPLICIT_DEPENDS feature.
  set(ZOT_DEPENDS IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/dep.cxx)
  set(ZOT_CUSTOM_DEP
    IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/dep_custom.cxx
                     CXX ${CMAKE_CURRENT_SOURCE_DIR}/dep_custom2.cxx )
else()
  # No IMPLICIT_DEPENDS...just depend directly.
  set(ZOT_DEPENDS DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/zot.hxx.in)
  set(ZOT_CUSTOM_DEP DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/zot_custom.hxx.in)
endif()
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zot.hxx
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_BINARY_DIR}/zot.hxx.in
  ${CMAKE_CURRENT_BINARY_DIR}/zot.hxx
  ${ZOT_DEPENDS}
  )

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zot_custom.hxx
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_BINARY_DIR}/zot_custom.hxx.in
  ${CMAKE_CURRENT_BINARY_DIR}/zot_custom.hxx
  ${ZOT_CUSTOM_DEP}
  )
add_custom_target(zot_custom ALL DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/zot_custom.hxx)

add_executable(zot zot.cxx ${CMAKE_CURRENT_BINARY_DIR}/zot.hxx
  zot_macro_dir.cxx zot_macro_tgt.cxx)
add_dependencies(zot zot_custom)

# Test the #include line macro transformation rule support.
set_property(
  TARGET zot
  PROPERTY IMPLICIT_DEPENDS_INCLUDE_TRANSFORM "ZOT_TGT(%)=<zot_%_tgt.hxx>"
  )

set_property(
  DIRECTORY
  PROPERTY IMPLICIT_DEPENDS_INCLUDE_TRANSFORM "ZOT_DIR(%)=<zot_%_dir.hxx>"
  )

if(TEST_LINK_DEPENDS)
  add_executable(linkdep linkdep.cxx)
  set_property(TARGET linkdep PROPERTY LINK_DEPENDS ${TEST_LINK_DEPENDS})
endif()

add_library(link_depends_no_shared_lib SHARED link_depends_no_shared_lib.c
  ${CMAKE_CURRENT_BINARY_DIR}/link_depends_no_shared_lib.h)

add_executable(link_depends_no_shared_exe_no_relink link_depends_no_shared_exe.c
  ${CMAKE_CURRENT_BINARY_DIR}/link_depends_no_shared_exe.h)
target_link_libraries(link_depends_no_shared_exe_no_relink link_depends_no_shared_lib)
set_property(TARGET link_depends_no_shared_exe_no_relink PROPERTY LINK_DEPENDS_NO_SHARED 1)

add_executable(link_depends_no_shared_exe_relink link_depends_no_shared_exe.c
  ${CMAKE_CURRENT_BINARY_DIR}/link_depends_no_shared_exe.h)
target_link_libraries(link_depends_no_shared_exe_relink link_depends_no_shared_lib)

add_custom_target(link_depends_no_shared_check ALL
  COMMAND ${CMAKE_COMMAND}
   -Dlib=$<TARGET_FILE:link_depends_no_shared_lib>
   -Dexe_relink=$<TARGET_FILE:link_depends_no_shared_exe_relink>
   -Dexe_no_relink=$<TARGET_FILE:link_depends_no_shared_exe_no_relink>
   -Dout=${CMAKE_CURRENT_BINARY_DIR}/link_depends_no_shared_check.txt
   -P ${CMAKE_CURRENT_SOURCE_DIR}/link_depends_no_shared_check.cmake
  )
add_dependencies(link_depends_no_shared_check
  link_depends_no_shared_exe_no_relink
  link_depends_no_shared_exe_relink
)
