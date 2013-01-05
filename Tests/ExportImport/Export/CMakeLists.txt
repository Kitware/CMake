cmake_minimum_required (VERSION 2.7.20090711)
project(Export C CXX)

# Pretend that RelWithDebInfo should link to debug libraries to test
# the DEBUG_CONFIGURATIONS property.
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug RelWithDebInfo)

add_library(testExe1lib STATIC testExe1lib.c) # not exported
add_executable(testExe1 testExe1.c)
target_link_libraries(testExe1 testExe1lib)
set_property(TARGET testExe1 PROPERTY VERSION 4)

add_library(testExe2libImp SHARED testExe2libImp.c)
set_property(TARGET testExe2libImp PROPERTY LIBRARY_OUTPUT_DIRECTORY impl)
add_library(testExe2lib SHARED testExe2lib.c)
target_link_libraries(testExe2lib testExe2libImp)
set_property(TARGET testExe2lib PROPERTY LINK_INTERFACE_LIBRARIES "")
add_executable(testExe2 testExe2.c)
set_property(TARGET testExe2 PROPERTY ENABLE_EXPORTS 1)
set_property(TARGET testExe2 PROPERTY LINK_INTERFACE_LIBRARIES testExe2lib)

add_library(testLib1 STATIC testLib1.c)
add_library(testLib2 STATIC testLib2.c)
target_link_libraries(testLib2 testLib1)

# Test library with empty link interface.  Link it to an implementation
# dependency that itself links to dependencies publicly.
add_library(testLib3ImpDep SHARED testLib3ImpDep.c)
set_property(TARGET testLib3ImpDep PROPERTY LIBRARY_OUTPUT_DIRECTORY impl/dep)
add_library(testLib3Imp SHARED testLib3Imp.c)
set_property(TARGET testLib3Imp PROPERTY LIBRARY_OUTPUT_DIRECTORY impl)
target_link_libraries(testLib3Imp testLib3ImpDep)
add_library(testLib3 SHARED testLib3.c)
target_link_libraries(testLib3 testLib3Imp)
set_property(TARGET testLib3 PROPERTY LINK_INTERFACE_LIBRARIES "")
set_property(TARGET testLib3 PROPERTY VERSION 1.2)
set_property(TARGET testLib3 PROPERTY SOVERSION 3)

# Test <ARCHIVE|LIBRARY|RUNTIME>_OUTPUT_NAME[_<CONFIG>] properties.
set_property(TARGET testLib3 PROPERTY RUNTIME_OUTPUT_NAME_DEBUG testLib3dll-d)
set_property(TARGET testLib3 PROPERTY RUNTIME_OUTPUT_NAME_RELEASE testLib3dll-r)
set_property(TARGET testLib3 PROPERTY RUNTIME_OUTPUT_NAME testLib3dll)
set_property(TARGET testLib3 PROPERTY LIBRARY_OUTPUT_NAME_DEBUG testLib3lib-d)
set_property(TARGET testLib3 PROPERTY LIBRARY_OUTPUT_NAME_RELEASE testLib3lib-r)
set_property(TARGET testLib3 PROPERTY LIBRARY_OUTPUT_NAME testLib3lib)
set_property(TARGET testLib3 PROPERTY ARCHIVE_OUTPUT_NAME testLib3import)

add_library(testLib4 SHARED testLib4.c)
set_property(TARGET testLib4 PROPERTY FRAMEWORK 1)

add_library(testLib5 SHARED testLib5.c)

add_library(testLib6 STATIC testLib6.cxx testLib6c.c)

# Work-around: Visual Studio 6 does not support per-target object files.
set(VS6)
if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")
  set(VS6 1)
endif()

# Test using the target_link_libraries command to set the
# LINK_INTERFACE_LIBRARIES* properties.  We construct two libraries
# providing the same two symbols.  In each library one of the symbols
# will work and the other one will fail to link.  The import part of
# this test will try to use the symbol corresponding to the
# configuration in which it is built.  If the proper library is not
# used via the link interface the import test will fail to link.
add_library(testLib4lib STATIC testLib4lib.c)
add_library(testLib4libdbg STATIC testLib4libopt.c testLib4libdbg${VS6}.c)
add_library(testLib4libopt STATIC testLib4libdbg.c testLib4libopt${VS6}.c)
set_property(TARGET testLib4libdbg PROPERTY COMPILE_DEFINITIONS LIB_DBG)
set_property(TARGET testLib4libopt PROPERTY COMPILE_DEFINITIONS LIB_OPT)
target_link_libraries(testLib4
  LINK_INTERFACE_LIBRARIES
  testLib4lib debug testLib4libdbg optimized testLib4libopt
  )

add_executable(testExe3 testExe3.c)
set_property(TARGET testExe3 PROPERTY MACOSX_BUNDLE 1)

# Test cyclic dependencies.
add_library(testLibCycleA STATIC
  testLibCycleA1.c testLibCycleA2.c testLibCycleA3.c)
add_library(testLibCycleB STATIC
  testLibCycleB1.c testLibCycleB2.c testLibCycleB3.c)
target_link_libraries(testLibCycleA testLibCycleB)
target_link_libraries(testLibCycleB testLibCycleA)
set_property(TARGET testLibCycleA PROPERTY LINK_INTERFACE_MULTIPLICITY 3)

# Test exporting dependent libraries into different exports
add_library(testLibRequired testLibRequired.c)
add_library(testLibDepends testLibDepends.c)
target_link_libraries(testLibDepends LINK_PUBLIC testLibRequired)

macro(add_include_lib _libName)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${_libName}.c" "// no content\n")
  add_library(${_libName} "${CMAKE_CURRENT_BINARY_DIR}/${_libName}.c")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_libName}")
  set_property(TARGET ${_libName} APPEND PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/${_libName}>"
        "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/${_libName}>"
      )
  if (NOT "${ARGV1}" STREQUAL "NO_HEADER")
      file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${_libName}/${_libName}.h" "// no content\n")
      install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${_libName}/${_libName}.h"
          DESTINATION include/${_libName}
      )
  endif()
endmacro()

add_include_lib(testLibIncludeRequired1)
add_include_lib(testLibIncludeRequired2)
add_include_lib(testLibIncludeRequired3 NO_HEADER)
# Generate testLibIncludeRequired4 in the testLibIncludeRequired3 directory
# with an error. If the includes from testLibIncludeRequired3 appear first,
# the error will be hit.
# Below, the '3' library appears before the '4' library
# but we are testing that the INSTALL_INTERFACE causes it not to be used
# at build time.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/testLibIncludeRequired3/testLibIncludeRequired4.h" "#error Should not be included\n")
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/testLibIncludeRequired3/testLibIncludeRequired4.h"
    DESTINATION include/testLibIncludeRequired3
)
add_include_lib(testLibIncludeRequired4)
add_include_lib(testLibIncludeRequired5 NO_HEADER)
# Generate testLibIncludeRequired6 in the testLibIncludeRequired5 directory
# with an error. If the includes from testLibIncludeRequired5 appear first,
# the error will be hit.
# Below, the '5' library appears before the '6' library
# but we are testing that when the installed IMPORTED target is used, from
# the Import side of this unit test, the '6' include from the '5' directory
# will not be used because it is in the BUILD_INTERFACE only.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/testLibIncludeRequired5/testLibIncludeRequired6.h" "#error Should not be included\n")
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/testLibIncludeRequired5/testLibIncludeRequired6.h"
    DESTINATION include/testLibIncludeRequired5
)
add_include_lib(testLibIncludeRequired6)

set_property(TARGET testLibRequired APPEND PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:testLibIncludeRequired1,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:$<1:$<TARGET_NAME:testLibIncludeRequired2>>,INTERFACE_INCLUDE_DIRECTORIES>
    $<INSTALL_INTERFACE:$<TARGET_PROPERTY:testLibIncludeRequired3,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:testLibIncludeRequired4,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:testLibIncludeRequired5,INTERFACE_INCLUDE_DIRECTORIES>>
    $<INSTALL_INTERFACE:$<TARGET_PROPERTY:testLibIncludeRequired6,INTERFACE_INCLUDE_DIRECTORIES>>
    # The BUILD_INTERFACE entry from above is duplicated below. This is to test that
    # the INSTALL_INTERFACE entry bound by a BUILD_INTERFACE entry on either side is
    # preprocessed correctly on install(EXPORT).
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:testLibIncludeRequired5,INTERFACE_INCLUDE_DIRECTORIES>>
    # Test that the below is non-fatal
    $<$<STREQUAL:one,two>:$<TARGET_PROPERTY:not_a_target,INTERFACE_INCLUDE_DIRECTORIES>>
)

set_property(TARGET testLibRequired APPEND PROPERTY
  INTERFACE_COMPILE_DEFINITIONS
    testLibRequired_IFACE_DEFINE
    $<BUILD_INTERFACE:BuildOnly_DEFINE>
    $<INSTALL_INTERFACE:InstallOnly_DEFINE>
)
include(GenerateExportHeader)

add_subdirectory(renamed)

add_library(testSharedLibRequired SHARED testSharedLibRequired.cpp)
generate_export_header(testSharedLibRequired)
set_property(TARGET testSharedLibRequired
  PROPERTY
    INTERFACE_POSITION_INDEPENDENT_CODE ON
)
set_property(TARGET testSharedLibRequired APPEND PROPERTY
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}"
)
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/testSharedLibRequired.h"
  "${CMAKE_CURRENT_BINARY_DIR}/testsharedlibrequired_export.h"
    DESTINATION include/testSharedLibRequired
)
set_property(TARGET testSharedLibRequired APPEND PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/testSharedLibRequired>"
                                "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}>"
)
set_property(TARGET testSharedLibRequired APPEND PROPERTY
  INTERFACE_COMPILE_DEFINITIONS USING_TESTSHAREDLIBREQUIRED
)
set_property(TARGET testSharedLibRequired
  APPEND PROPERTY
    COMPATIBLE_INTERFACE_BOOL CUSTOM_PROP
)
set_property(TARGET testSharedLibRequired
  PROPERTY
    INTERFACE_CUSTOM_PROP ON
)
set_property(TARGET testSharedLibRequired
  APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING CUSTOM_STRING
)
set_property(TARGET testSharedLibRequired
  PROPERTY
    INTERFACE_CUSTOM_STRING testcontent
)
set_property(TARGET testSharedLibRequired APPEND PROPERTY
  INTERFACE_COMPILE_OPTIONS
    $<$<CXX_COMPILER_ID:GNU>:-DCUSTOM_COMPILE_OPTION>
)


add_library(testSharedLibDepends SHARED testSharedLibDepends.cpp)
set_property(TARGET testSharedLibDepends APPEND PROPERTY
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}"
)
generate_export_header(testSharedLibDepends)

set_property(TARGET testSharedLibDepends APPEND PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:testSharedLibRequired,INTERFACE_INCLUDE_DIRECTORIES>
)
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/testSharedLibDepends.h"
  "${CMAKE_CURRENT_BINARY_DIR}/testsharedlibdepends_export.h"
    DESTINATION include/testSharedLibDepends
)
set_property(TARGET testSharedLibDepends APPEND PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/testSharedLibDepends>"
                                "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}>"
)

# LINK_PRIVATE because the LINK_INTERFACE_LIBRARIES is specified above.
target_link_libraries(testSharedLibDepends LINK_PRIVATE testSharedLibRequired)
target_link_libraries(testSharedLibDepends LINK_PUBLIC renamed_on_export)
target_link_libraries(testSharedLibDepends LINK_INTERFACE_LIBRARIES
  $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:$<TARGET_NAME:testSharedLibRequired>>)

add_library(cmp0022OLD SHARED cmp0022_vs6_1.cpp)
generate_export_header(cmp0022OLD BASE_NAME cmp0022)
target_include_directories(cmp0022OLD PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/cmp0022>"
)
cmake_policy(SET CMP0022 NEW)
add_library(cmp0022NEW SHARED cmp0022_vs6_1.cpp)
target_include_directories(cmp0022NEW PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/cmp0022>"
)
cmake_policy(SET CMP0022 OLD)
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/cmp0022.h"
  "${CMAKE_CURRENT_BINARY_DIR}/cmp0022_export.h"
    DESTINATION include/cmp0022
)

set_property(TARGET testLib2 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS USING_TESTLIB2)
set_property(TARGET testLib3 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS USING_TESTLIB3)

set_property(TARGET cmp0022NEW APPEND PROPERTY INTERFACE_LINK_LIBRARIES testLib2)
# set_property(TARGET cmp0022NEW APPEND PROPERTY LINK_INTERFACE_LIBRARIES testLibIncludeRequired2) # TODO: Test for error
set_property(TARGET cmp0022OLD APPEND PROPERTY INTERFACE_LINK_LIBRARIES testLib2)
set_property(TARGET cmp0022OLD APPEND PROPERTY LINK_INTERFACE_LIBRARIES testLib3)

install(TARGETS testLibRequired
                testLibIncludeRequired1
                testLibIncludeRequired2
                testLibIncludeRequired3
                testLibIncludeRequired4
                testLibIncludeRequired5
                testLibIncludeRequired6
                testSharedLibRequired
        EXPORT RequiredExp DESTINATION lib
        INCLUDES DESTINATION
          installIncludesTest
          $<INSTALL_PREFIX>/installIncludesTest2)
install(EXPORT RequiredExp NAMESPACE Req:: FILE testLibRequiredTargets.cmake DESTINATION lib/cmake/testLibRequired)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest/installIncludesTest.h" "// No content\n")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest2")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest2/installIncludesTest2.h" "// No content\n")
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest/installIncludesTest.h"
  DESTINATION installIncludesTest
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest2/installIncludesTest2.h"
  DESTINATION installIncludesTest2
)

install(TARGETS testLibDepends testSharedLibDepends EXPORT DependsExp DESTINATION lib )
install(EXPORT DependsExp FILE testLibDependsTargets.cmake DESTINATION lib/cmake/testLibDepends)

file(WRITE
  "${CMAKE_CURRENT_BINARY_DIR}/testLibRequiredConfig.cmake"
  "
if(\${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION VERSION_LESS 2.3 AND NOT \${CMAKE_FIND_PACKAGE_NAME}_INTERFACES)
  set(\${CMAKE_FIND_PACKAGE_NAME}_NO_INTERFACES 1)
endif()
include(\"\${CMAKE_CURRENT_LIST_DIR}/testLibRequiredTargets.cmake\")
set(\${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIRS \"${CMAKE_CURRENT_BINARY_DIR}\" \"${CMAKE_CURRENT_SOURCE_DIR}\" )
"
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file( testLibRequiredConfigVersion.cmake VERSION 2.5 COMPATIBILITY AnyNewerVersion)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/testLibRequiredConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/testLibRequiredConfigVersion.cmake"
  DESTINATION lib/cmake/testLibRequired
)

# Install and export from install tree.
install(
  TARGETS
  testExe1 testLib1 testLib2 testExe2 testLib3 testLib4 testExe3
  testExe2lib testLib4lib testLib4libdbg testLib4libopt
  testLib6
  testLibCycleA testLibCycleB
  cmp0022NEW cmp0022OLD
  EXPORT exp
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib NAMELINK_SKIP
  ARCHIVE DESTINATION lib
  FRAMEWORK DESTINATION Frameworks
  BUNDLE DESTINATION Applications
  )
install(
  TARGETS
  testExe2libImp testLib3Imp
  EXPORT exp
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib/impl
  ARCHIVE DESTINATION lib/impl
  )
install(
  TARGETS
  testLib3ImpDep
  EXPORT exp
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib/impl/dep
  ARCHIVE DESTINATION lib/impl/dep
  )
install(
  TARGETS testLib5
  EXPORT exp
  # Leave out RUNTIME DESTINATION to test implib-only export.
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
install(EXPORT exp NAMESPACE exp_ DESTINATION lib/exp)

# Install testLib5.dll outside the export.
if(WIN32)
  install(TARGETS testLib5 RUNTIME DESTINATION bin)
endif()

add_subdirectory(sublib) # For CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE test.

# Export from build tree.
export(TARGETS testExe1 testLib1 testLib2 testLib3
  testExe2libImp testLib3Imp testLib3ImpDep subdirlib
  testSharedLibRequired testSharedLibDepends renamed_on_export
  cmp0022NEW cmp0022OLD
  NAMESPACE bld_
  FILE ExportBuildTree.cmake
  )
export(TARGETS testExe2 testLib4 testLib5 testLib6 testExe3 testExe2lib
  testLib4lib testLib4libdbg testLib4libopt
  testLibCycleA testLibCycleB
  NAMESPACE bld_
  APPEND FILE ExportBuildTree.cmake
  )
