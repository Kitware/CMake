set(CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES "1942b4fa-b2c5-4546-9385-83f254070067")
cmake_minimum_required(VERSION 3.10)
if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()
project(Export C CXX)

set(CMAKE_AIX_SHARED_LIBRARY_ARCHIVE 1)

find_package(Foo REQUIRED CONFIG NO_DEFAULT_PATH)

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
add_executable(testExe2 testExe2.c)
set_property(TARGET testExe2 PROPERTY ENABLE_EXPORTS 1)
set_property(TARGET testExe2 PROPERTY INTERFACE_LINK_LIBRARIES testExe2lib)

add_library(compileOnly INTERFACE)
target_compile_definitions(compileOnly INTERFACE FROM_compileOnly)
target_link_options(compileOnly INTERFACE -fthis-flag-does-not-exist)

add_library(noUses INTERFACE)
target_link_libraries(noUses INTERFACE this::target_does_not_exist)

add_library(testLib1 STATIC testLib1.c)
add_library(testLib2 STATIC testLib2.c)
target_link_libraries(testLib2
  PRIVATE
    testLib1
    "$<COMPILE_ONLY:compileOnly>")


# Test install(FILES) with generator expressions referencing testLib1.
add_custom_command(TARGET testLib1 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:testLib1>
                                   $<TARGET_FILE:testLib1>.genex
  )
install(FILES $<TARGET_FILE:testLib1>.genex
  DESTINATION $<1:lib>$<0:/wrong>
  )
set_property(TARGET testLib1 PROPERTY MY_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/testLib1file1.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/testLib1file2.txt
  )
install(FILES $<TARGET_PROPERTY:testLib1,MY_FILES>
  DESTINATION $<1:doc>$<0:/wrong>
  )

# Test library with empty link interface.  Link it to an implementation
# dependency that itself links to dependencies publicly.
add_library(testLib3ImpDep SHARED testLib3ImpDep.c)
set_property(TARGET testLib3ImpDep PROPERTY LIBRARY_OUTPUT_DIRECTORY impl/dep)
add_library(testLib3Imp SHARED testLib3Imp.c)
set_property(TARGET testLib3Imp PROPERTY LIBRARY_OUTPUT_DIRECTORY impl)
target_link_libraries(testLib3Imp testLib3ImpDep)
add_library(testLib3 SHARED testLib3.c)
target_link_libraries(testLib3 testLib3Imp)
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

add_library(testLibPerConfigDest STATIC testLibPerConfigDest.c)
install(TARGETS testLibPerConfigDest EXPORT exp
  DESTINATION lib/$<$<BOOL:$<CONFIG>>:$<CONFIG>>$<$<NOT:$<BOOL:$<CONFIG>>>:NoConfig>
  )

# Test OUTPUT_NAME properties with generator expressions
add_library(testLib7 STATIC testLib7.c)
set_property(TARGET testLib7 PROPERTY OUTPUT_NAME_DEBUG testLib7D-$<CONFIG>)
set_property(TARGET testLib7 PROPERTY OUTPUT_NAME_RELEASE testLib7R-$<CONFIG>)
set_property(TARGET testLib7 PROPERTY OUTPUT_NAME testLib7-$<CONFIG>)

# Test exporting OBJECT targets
add_library(testLib8 OBJECT testLib8A.c testLib8B.c sub/testLib8C.c)

if(NOT CMAKE_GENERATOR STREQUAL "Xcode" OR NOT CMAKE_OSX_ARCHITECTURES MATCHES "[;$]")
  set(maybe_OBJECTS_DESTINATION OBJECTS DESTINATION $<1:lib>)
else()
  set(maybe_OBJECTS_DESTINATION "")
endif()

add_library(testLib9ObjPub OBJECT testLib9ObjPub.c)
target_compile_definitions(testLib9ObjPub INTERFACE testLib9ObjPub_USED)
add_library(testLib9ObjPriv OBJECT testLib9ObjPriv.c)
target_compile_definitions(testLib9ObjPriv INTERFACE testLib9ObjPriv_USED)
add_library(testLib9ObjIface OBJECT testLib9ObjIface.c)
target_compile_definitions(testLib9ObjIface INTERFACE testLib9ObjIface_USED)
add_library(testLib9 STATIC testLib9.c)
target_link_libraries(testLib9 INTERFACE testLib9ObjIface PUBLIC testLib9ObjPub PRIVATE testLib9ObjPriv)
target_link_libraries(testLib9 PUBLIC Foo::Foo)

add_library(testLib10 STATIC testLib10.c)
set_target_properties(testLib10 PROPERTIES
  TRANSITIVE_COMPILE_PROPERTIES "CUSTOM_C"
  TRANSITIVE_LINK_PROPERTIES    "CUSTOM_L"
  INTERFACE_CUSTOM_C "TESTLIB10_INTERFACE_CUSTOM_C"
  INTERFACE_CUSTOM_L "TESTLIB10_INTERFACE_CUSTOM_L"
  )
target_compile_definitions(testLib10 INTERFACE
  "$<TARGET_PROPERTY:CUSTOM_C>"
  "$<TARGET_PROPERTY:CUSTOM_L>"
  )
add_library(testLib11 STATIC testLib11.c)
target_link_libraries(testLib11 PRIVATE testLib10)
set_target_properties(testLib11 PROPERTIES
  INTERFACE_CUSTOM_C "TESTLIB11_INTERFACE_CUSTOM_C"
  INTERFACE_CUSTOM_L "TESTLIB11_INTERFACE_CUSTOM_L"
  TRANSITIVE_COMPILE_PROPERTIES "CUSTOM_D"
  TRANSITIVE_LINK_PROPERTIES    "CUSTOM_M"
  INTERFACE_CUSTOM_D "TESTLIB11_INTERFACE_CUSTOM_D"
  INTERFACE_CUSTOM_M "TESTLIB11_INTERFACE_CUSTOM_M"
  )
target_compile_definitions(testLib11 INTERFACE
  "$<TARGET_PROPERTY:CUSTOM_C>"
  "$<TARGET_PROPERTY:CUSTOM_D>"
  "$<TARGET_PROPERTY:CUSTOM_L>"
  "$<TARGET_PROPERTY:CUSTOM_M>"
  )

# Test using the target_link_libraries command to set the
# INTERFACE_LINK_LIBRARIES properties.  We construct two libraries
# providing the same two symbols.  In each library one of the symbols
# will work and the other one will fail to link.  The import part of
# this test will try to use the symbol corresponding to the
# configuration in which it is built.  If the proper library is not
# used via the link interface the import test will fail to link.
add_library(testLib4lib STATIC testLib4lib.c)
add_library(testLib4libdbg STATIC testLib4libopt.c testLib4libdbg.c)
add_library(testLib4libopt STATIC testLib4libdbg.c testLib4libopt.c)
set_property(TARGET testLib4libdbg PROPERTY COMPILE_DEFINITIONS LIB_DBG)
set_property(TARGET testLib4libopt PROPERTY COMPILE_DEFINITIONS LIB_OPT)
target_link_libraries(testLib4
  LINK_INTERFACE_LIBRARIES
  testLib4lib debug $<TARGET_NAME:testLib4libdbg> optimized $<TARGET_NAME:testLib4libopt>
  )

add_executable(testExe3 testExe3.c)
set_property(TARGET testExe3 PROPERTY MACOSX_BUNDLE 1)

# Test <ARCHIVE|LIBRARY|RUNTIME>_OUTPUT_DIRECTORY[_<CONFIG>] properties with generator expressions
add_executable(testExe4 testExe4.c)
target_link_libraries(testExe4 testExe1lib)
set_property(TARGET testLib7 PROPERTY ARCHIVE_OUTPUT_DIRECTORY_DEBUG testLib7D-$<CONFIG>)
set_property(TARGET testLib7 PROPERTY ARCHIVE_OUTPUT_DIRECTORY_RELEASE testLib7R-$<CONFIG>)
set_property(TARGET testLib7 PROPERTY ARCHIVE_OUTPUT_DIRECTORY testLib7-$<CONFIG>)
set_property(TARGET testLib5 PROPERTY LIBRARY_OUTPUT_DIRECTORY_DEBUG testLib5D-$<CONFIG>)
set_property(TARGET testLib5 PROPERTY LIBRARY_OUTPUT_DIRECTORY_RELEASE testLib5R-$<CONFIG>)
set_property(TARGET testLib5 PROPERTY LIBRARY_OUTPUT_DIRECTORY testLib5-$<CONFIG>)
set_property(TARGET testExe4 PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG testExe4D-$<CONFIG>)
set_property(TARGET testExe4 PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE testExe4R-$<CONFIG>)
set_property(TARGET testExe4 PROPERTY RUNTIME_OUTPUT_DIRECTORY testExe4-$<CONFIG>)

# Test cyclic dependencies.
add_library(testLibCycleA STATIC
  testLibCycleA1.c testLibCycleA2.c testLibCycleA3.c)
add_library(testLibCycleB STATIC
  testLibCycleB1.c testLibCycleB2.c testLibCycleB3.c)
target_link_libraries(testLibCycleA testLibCycleB)
target_link_libraries(testLibCycleB testLibCycleA)
set_property(TARGET testLibCycleA PROPERTY LINK_INTERFACE_MULTIPLICITY 3)

add_library(testLibNoSONAME SHARED testLibNoSONAME.c)
set_property(TARGET testLibNoSONAME PROPERTY NO_SONAME 1)

add_library(testInterfaceIncludeUser INTERFACE)
target_include_directories(testInterfaceIncludeUser
  INTERFACE
    "$<INSTALL_INTERFACE:include/testInterfaceIncludeUser>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/testInterfaceIncludeUser>"
)
set_property(TARGET testInterfaceIncludeUser PROPERTY IMPORTED_NO_SYSTEM 1)

add_library(testInterfaceIncludeUser2 INTERFACE)
target_include_directories(testInterfaceIncludeUser2
  INTERFACE
    "$<INSTALL_INTERFACE:include/testInterfaceIncludeUser>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/testInterfaceIncludeUser>"
)
set_property(TARGET testInterfaceIncludeUser2 PROPERTY EXPORT_NO_SYSTEM 1)

install(
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/testInterfaceIncludeUser/testInterfaceInclude.h"
  DESTINATION include/testInterfaceIncludeUser
)

# Test control over direct linking.
include(../../InterfaceLinkLibrariesDirect/testStaticLibPlugin.cmake)
include(../../InterfaceLinkLibrariesDirect/testSharedLibWithHelper.cmake)
include(../../InterfaceLinkLibrariesDirect/testExeWithPluginHelper.cmake)
if(NOT maybe_OBJECTS_DESTINATION)
  target_compile_definitions(testSharedLibHelperObj INTERFACE testSharedLibHelperObj_NO_OBJECT)
  target_compile_definitions(testExePluginHelperObj INTERFACE testExePluginHelperObj_NO_OBJECT)
endif()

# Test exporting dependent libraries into different exports
add_library(testLibRequired testLibRequired.c)
add_library(testLibDepends testLibDepends.c)
target_link_libraries(testLibDepends LINK_PUBLIC testLibRequired)
add_library(testStaticLibRequiredPrivate testStaticLibRequiredPrivate.c)
target_link_libraries(testLibDepends PRIVATE testStaticLibRequiredPrivate)

cmake_policy(PUSH)
cmake_policy(SET CMP0079 NEW)
add_library(TopDirLib STATIC testTopDirLib.c)
add_subdirectory(SubDirLinkA)
add_subdirectory(SubDirLinkB)
cmake_policy(POP)

macro(add_include_lib _libName)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${_libName}.c" "/* no content */\n")
  add_library(${_libName} "${CMAKE_CURRENT_BINARY_DIR}/${_libName}.c")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_libName}")
  set_property(TARGET ${_libName} APPEND PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/${_libName}>"
      )
  if (NOT "${ARGV1}" STREQUAL "NO_HEADER")
      file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${_libName}/${_libName}.h" "/* no content */\n")
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

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/testLibIncludeRequired7/testLibIncludeRequired7.h" "/* No content */\n")
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/testLibIncludeRequired7/testLibIncludeRequired7.h"
    DESTINATION include/testLibIncludeRequired7
)

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
    $<INSTALL_INTERFACE:include/testLibIncludeRequired7;include/testLibIncludeRequired4>
)

set_property(TARGET testLibRequired APPEND PROPERTY
  INTERFACE_COMPILE_DEFINITIONS
    testLibRequired_IFACE_DEFINE
    $<BUILD_INTERFACE:BuildOnly_DEFINE>
    $<INSTALL_INTERFACE:InstallOnly_DEFINE>
)
include(GenerateExportHeader)

# Test deprecation of imported library
add_library(testLibDeprecation STATIC testLib1.c)
set_property(TARGET testLibDeprecation PROPERTY DEPRECATION "Deprecated version. Please use latest version")

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
  INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}>"
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
    $<$<CXX_COMPILER_ID:LCC>:-DCUSTOM_COMPILE_OPTION>
)

add_library(testSharedLibRequiredUser SHARED testSharedLibRequiredUser.cpp)
generate_export_header(testSharedLibRequiredUser)
# LINK_PRIVATE so that it appears in the LINK_DEPENDENT_LIBRARIES, but not
# the INTERFACE_LINK_LIBRARIES.
set_property(TARGET testSharedLibRequiredUser APPEND PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}>"
)
target_link_libraries(testSharedLibRequiredUser LINK_PRIVATE testSharedLibRequired)
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/testSharedLibRequiredUser.h"
  "${CMAKE_CURRENT_BINARY_DIR}/testsharedlibrequireduser_export.h"
    DESTINATION include/testSharedLibRequiredUser
)

add_library(testSharedLibRequiredUser2 SHARED testSharedLibRequiredUser2.cpp)
generate_export_header(testSharedLibRequiredUser2)
set_property(TARGET testSharedLibRequiredUser2 APPEND PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}>"
)
set_property(TARGET testSharedLibRequiredUser2 PROPERTY LINK_LIBRARIES testSharedLibRequired)
set_property(TARGET testSharedLibRequiredUser2 PROPERTY INTERFACE_LINK_LIBRARIES testSharedLibRequired)
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/testSharedLibRequiredUser2.h"
  "${CMAKE_CURRENT_BINARY_DIR}/testsharedlibrequireduser2_export.h"
    DESTINATION include/testSharedLibRequiredUser2
)

cmake_policy(PUSH)
cmake_policy(SET CMP0041 NEW)
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
  INTERFACE_INCLUDE_DIRECTORIES "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/$<1:include>/testSharedLibDepends>"
                                "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}>"
)
cmake_policy(POP)

target_link_libraries(testSharedLibDepends LINK_PRIVATE testSharedLibRequired)
target_link_libraries(testSharedLibDepends LINK_PUBLIC renamed_on_export)
target_link_libraries(testSharedLibDepends INTERFACE
  $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:$<TARGET_NAME:testSharedLibRequired>>)

add_library(cmp0022NEW SHARED cmp0022_vs6_1.cpp)
generate_export_header(cmp0022NEW BASE_NAME cmp0022)
target_include_directories(cmp0022NEW PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/cmp0022>"
)
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/cmp0022.h"
  "${CMAKE_CURRENT_BINARY_DIR}/cmp0022_export.h"
    DESTINATION include/cmp0022
)

set_property(TARGET testLib2 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS USING_TESTLIB2)
set_property(TARGET testLib3 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS USING_TESTLIB3)
set_target_properties(testLib3 PROPERTIES
  EXPORT_PROPERTIES "EXPORTED_PROPERTY1"
  EXPORTED_PROPERTY1 "EXPORTING_TESTLIB3")
set_target_properties(testLib4 PROPERTIES
  EXPORTED_PROPERTY2 "EXPORTING_TESTLIB4_1"
  EXPORTED_PROPERTY3 "EXPORTING_TESTLIB4_2")
set_property(TARGET testLib4 PROPERTY
  EXPORT_PROPERTIES EXPORTED_PROPERTY2 EXPORTED_PROPERTY3)

set_property(TARGET cmp0022NEW APPEND PROPERTY INTERFACE_LINK_LIBRARIES testLib2)

add_library(noIncludesInterface empty.cpp)

add_library(systemlib SHARED systemlib.cpp)
install(FILES systemlib.h DESTINATION include/systemlib)
target_include_directories(systemlib
  INTERFACE
    $<INSTALL_INTERFACE:include/systemlib>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_library(testMod1 MODULE empty.cpp)
add_library(testMod2 MODULE empty.cpp)
set_property(TARGET testMod2 PROPERTY BUNDLE 1)

add_library(testSharedObjectAIX SHARED testSharedObjectAIX.c)
set_property(TARGET testSharedObjectAIX PROPERTY AIX_SHARED_LIBRARY_ARCHIVE 0)

install(TARGETS testLibRequired
        EXPORT RequiredExp DESTINATION lib
        INCLUDES DESTINATION
          installIncludesTest
          $<INSTALL_PREFIX>/installIncludesTest2
          installIncludesTest3/$<TARGET_PROPERTY:NAME>
          $<TARGET_PROPERTY:NAME>/installIncludesTest4
          $<INSTALL_INTERFACE:installIncludesTest5$<0:>>
          $<INSTALL_INTERFACE:$<0:>installIncludesTest6>
          $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/installIncludesTest7>
)

target_include_directories(testLibRequired INTERFACE
  # These can't be in the above install(INCLUDES DESTINATION call because
  # that is only for installed interfaces. These directories are prefixes
  # in the build dir, which is an error for the installed interface.
  # We add them here so that we don't have to add conditions in the Import
  # component of the test.
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest5$<0:>>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/$<0:>installIncludesTest6>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest7>
  $<INSTALL_INTERFACE:installIncludesTest8/$<0:>>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest8$<0:>>
)
install(TARGETS
          testLibIncludeRequired1
          testLibIncludeRequired2
          testLibIncludeRequired3
          testLibIncludeRequired4
          testLibIncludeRequired5
          testLibIncludeRequired6
          testSharedLibRequired
          testSharedLibRequiredUser
          testSharedLibRequiredUser2
          noIncludesInterface
        EXPORT RequiredExp DESTINATION lib
        INCLUDES DESTINATION
          $<INSTALL_PREFIX>/include/$<TARGET_PROPERTY:NAME>
)
install(TARGETS
          testStaticLibRequiredPrivate
        EXPORT RequiredExp DESTINATION lib
)
install(EXPORT RequiredExp NAMESPACE Req:: FILE testLibRequiredTargets.cmake DESTINATION lib/cmake/testLibRequired)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest/installIncludesTest.h" "/* No content */\n")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest2")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest2/installIncludesTest2.h" "/* No content */\n")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest3/testLibRequired")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest3/testLibRequired/installIncludesTest3.h" "/* No content */\n")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/testLibRequired/installIncludesTest4")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/testLibRequired/installIncludesTest4/installIncludesTest4.h" "/* No content */\n")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest5")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest5/installIncludesTest5.h" "/* No content */\n")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest6")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest6/installIncludesTest6.h" "/* No content */\n")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest7")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest7/installIncludesTest7.h" "/* No content */\n")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest8")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest8/installIncludesTest8.h" "/* No content */\n")
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest/installIncludesTest.h"
  DESTINATION installIncludesTest
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest2/installIncludesTest2.h"
  DESTINATION installIncludesTest2
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest3/testLibRequired/installIncludesTest3.h"
  DESTINATION installIncludesTest3/testLibRequired
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/testLibRequired/installIncludesTest4/installIncludesTest4.h"
  DESTINATION testLibRequired/installIncludesTest4
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest5/installIncludesTest5.h"
  DESTINATION installIncludesTest5
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest6/installIncludesTest6.h"
  DESTINATION installIncludesTest6
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest7/installIncludesTest7.h"
  DESTINATION installIncludesTest7
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/installIncludesTest8/installIncludesTest8.h"
  DESTINATION installIncludesTest8
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
  compileOnly
  noUses
  testExe1 testLib1 testLib2 testExe2 testLib3 testLib4 testExe3 testExe4
  testExe2lib testLib4lib testLib4libdbg testLib4libopt
  testLib6 testLib7 testLib8
  testLib9
  testLib10 testLib11
  testLibDeprecation
  testLibCycleA testLibCycleB
  testLibNoSONAME
  testStaticLibWithPlugin testStaticLibPluginExtra testStaticLibPlugin
  testSharedLibWithHelper testSharedLibHelperObj
  testExeWithPluginHelper testExePluginHelperObj
  testMod1 testMod2
  cmp0022NEW
  TopDirLib SubDirLinkA
  systemlib
  testInterfaceIncludeUser
  testInterfaceIncludeUser2
  testSharedObjectAIX
  EXPORT exp
  RUNTIME DESTINATION $<1:bin>$<0:/wrong>
  LIBRARY DESTINATION $<1:lib>$<0:/wrong> NAMELINK_SKIP
  ARCHIVE DESTINATION $<1:lib>$<0:/wrong>
  ${maybe_OBJECTS_DESTINATION}
  FRAMEWORK DESTINATION Frameworks
  BUNDLE DESTINATION Applications
  )
install(
  TARGETS
  testLib9ObjPub testLib9ObjPriv testLib9ObjIface
  EXPORT exp
  )
if (APPLE)
  file(COPY testLib4.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/testLib4.framework/Headers)
  file(COPY testLib4.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug/testLib4.framework/Headers)
  file(COPY testLib4.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/testLib4.framework/Headers)
  install(FILES testLib4.h DESTINATION Frameworks/testLib4.framework/Headers)
endif()
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
install(EXPORT exp NAMESPACE exp_ DESTINATION lib/exp EXPORT_PACKAGE_DEPENDENCIES)

# Install testLib5.dll outside the export.
if(WIN32)
  install(TARGETS testLib5 RUNTIME DESTINATION bin)
endif()

add_subdirectory(sublib) # For CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE test.

# Export from build tree.
export(TARGETS testExe1 testLib1 testLib2 testLib3
  testExe2libImp testLib3Imp testLib3ImpDep subdirlib
  testSharedLibRequired testSharedLibRequiredUser testSharedLibRequiredUser2
  testSharedLibDepends renamed_on_export
  cmp0022NEW
  TopDirLib SubDirLinkA
  systemlib noUses
  testInterfaceIncludeUser
  testInterfaceIncludeUser2
  NAMESPACE bld_
  FILE ExportBuildTree.cmake
  )
export(TARGETS testExe2 testLib4 testLib5 testLib6 testLib7 testExe3 testExe4 testExe2lib
  testLib8
  testLib9 testLib9ObjPub testLib9ObjPriv testLib9ObjIface
  testLib10 testLib11
  testLibDeprecation
  testLib4lib testLib4libdbg testLib4libopt
  testLibCycleA testLibCycleB
  testLibNoSONAME
  testStaticLibWithPlugin testStaticLibPluginExtra testStaticLibPlugin
  testSharedLibWithHelper testSharedLibHelperObj
  testExeWithPluginHelper testExePluginHelperObj
  testMod1 testMod2
  testLibPerConfigDest
  testSharedObjectAIX
  NAMESPACE bld_
  APPEND FILE ExportBuildTree.cmake
  )

add_subdirectory(Interface)

#-----------------------------------------------------------------------------
# Install export with absolute destination but relative pieces.
add_library(testLibAbs1 STATIC testLibAbs1.c)
target_include_directories(testLibAbs1 INTERFACE
  "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/abs/1a;include/abs/1b>"
  )
install(
  TARGETS testLibAbs1
  EXPORT expAbs
  ARCHIVE DESTINATION lib
  INCLUDES DESTINATION include/abs
  )
install(DIRECTORY $<1:include/abs>$<0:/wrong> DESTINATION $<1:include>$<0:/wrong>)
install(EXPORT expAbs NAMESPACE expAbs_ DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/expAbs)


#------------------------------------------------------------------------------
# test export of INTERFACE_LINK_OPTIONS
add_library(testLinkOptions INTERFACE)
target_link_options(testLinkOptions INTERFACE INTERFACE_FLAG)

install(TARGETS testLinkOptions
        EXPORT RequiredExp DESTINATION lib)
export(TARGETS testLinkOptions NAMESPACE bld_ APPEND FILE ExportBuildTree.cmake)


#------------------------------------------------------------------------------
# test export of INTERFACE_LINK_DIRECTORIES
add_library(testLinkDirectories INTERFACE)
target_link_directories(testLinkDirectories INTERFACE
  $<BUILD_INTERFACE:/interface/build>
  $<INSTALL_INTERFACE:interface/install>)

install(TARGETS testLinkDirectories
        EXPORT RequiredExp DESTINATION lib)
export(TARGETS testLinkDirectories NAMESPACE bld_ APPEND FILE ExportBuildTree.cmake)


#------------------------------------------------------------------------------
# test export of INTERFACE_LINK_DEPENDS
if(CMAKE_GENERATOR MATCHES "Make|Ninja")
  add_library(testLinkDepends INTERFACE)
  set_property(TARGET testLinkDepends PROPERTY INTERFACE_LINK_DEPENDS
    $<BUILD_INTERFACE:BUILD_LINK_DEPENDS>
    $<INSTALL_INTERFACE:INSTALL_LINK_DEPENDS>)

  install(TARGETS testLinkDepends
    EXPORT RequiredExp DESTINATION lib)
  export(TARGETS testLinkDepends NAMESPACE bld_ APPEND FILE ExportBuildTree.cmake)
endif()

#------------------------------------------------------------------------------
# test export of CUDA language
if(CMake_TEST_CUDA)
  enable_language(CUDA)
  add_library(cudaInterfaceLib INTERFACE)
  target_compile_features(cudaInterfaceLib INTERFACE $<BUILD_INTERFACE:cuda_std_11> $<INSTALL_INTERFACE:cuda_std_14>)

  install(TARGETS cudaInterfaceLib
    EXPORT RequiredExp DESTINATION lib)
  export(TARGETS cudaInterfaceLib NAMESPACE bld_ APPEND FILE ExportBuildTree.cmake)
endif()

# Test the presence of targets named the same as languages.
# IMPORTED_LINK_INTERFACE_LANGUAGES entries should not be targets.
add_library(C INTERFACE)
add_library(CXX INTERFACE)

#------------------------------------------------------------------------------
# test export of targets built from sources named using $<CONFIG> generator-expression
# FIXME: Enable test on Xcode generator when it supports per-config sources.
if(NOT XCODE)
  file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Generated_$<CONFIG>.cpp" CONTENT "// content")
  add_library(testStaticFromGeneratedSource STATIC)
  target_sources(testStaticFromGeneratedSource PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/Generated_$<CONFIG>.cpp")

  add_library(testLibFromGeneratedSource SHARED empty.cpp)
  target_link_libraries(testLibFromGeneratedSource PRIVATE testStaticFromGeneratedSource)

  install(TARGETS testLibFromGeneratedSource EXPORT testLibFromGeneratedSource_Export)
  install(EXPORT testLibFromGeneratedSource_Export DESTINATION lib)
endif()

add_subdirectory(install-RUNTIME_DEPENDENCY_SET)
