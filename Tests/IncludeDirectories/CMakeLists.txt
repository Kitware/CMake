cmake_minimum_required (VERSION 2.6)

if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()

project(IncludeDirectories)
if (((CMAKE_C_COMPILER_ID STREQUAL GNU AND CMAKE_C_COMPILER_VERSION VERSION_GREATER 4.4)
    OR (CMAKE_C_COMPILER_ID STREQUAL Clang AND NOT "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
    OR CMAKE_C_COMPILER_ID STREQUAL NVHPC
    OR CMAKE_C_COMPILER_ID STREQUAL AppleClang
    OR CMAKE_C_COMPILER_ID STREQUAL LCC
    OR ("x${CMAKE_C_COMPILER_ID}" STREQUAL "xMSVC" AND
       CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL "19.29.30036.3"))
    AND (CMAKE_GENERATOR STREQUAL "Unix Makefiles"
      OR CMAKE_GENERATOR STREQUAL "Ninja"
      OR (CMAKE_GENERATOR STREQUAL "Xcode" AND NOT XCODE_VERSION VERSION_LESS 6.0)
      OR CMAKE_GENERATOR MATCHES "Visual Studio"))
  if ("x${CMAKE_C_COMPILER_ID}" STREQUAL "xMSVC")
    set(run_sys_includes_test 1)
  else ()
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-Wunused-variable run_sys_includes_test)
    if(run_sys_includes_test)
      # The Bullseye wrapper appears to break the -isystem effect.
      execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE out ERROR_VARIABLE out)
      if("x${out}" MATCHES "Bullseye")
        set(run_sys_includes_test 0)
      endif()
    endif()
  endif()
  if (run_sys_includes_test)
    add_subdirectory(SystemIncludeDirectories)
    add_subdirectory(SystemIncludeDirectoriesPerLang)
  endif()
endif()

file(WRITE ${CMAKE_BINARY_DIR}/Flags/Flags.h
"//Flags.h
")
file(WRITE ${CMAKE_BINARY_DIR}/IncDir/IncDir.h
"//IncDir.h
")
file(WRITE ${CMAKE_BINARY_DIR}/SrcProp/SrcProp.h
"//SrcProp.h
")
file(WRITE ${CMAKE_BINARY_DIR}/TarProp/TarProp.h
"//TarProp.h
")

# default to testing with full path
# some compilers can not handle the escape for directories
# with spaces in them.
set(USE_FULLPATH TRUE)
if(WATCOM OR MSVC60)
  set(USE_FULLPATH FALSE)
endif()
if(USE_FULLPATH)
  string(APPEND CMAKE_CXX_FLAGS " \"-I${CMAKE_BINARY_DIR}/Flags\"")
else()
  string(APPEND CMAKE_CXX_FLAGS " -IFlags")
endif()

include_directories(${CMAKE_BINARY_DIR}/IncDir)
if(USE_FULLPATH)
  set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS
    "\"-I${CMAKE_BINARY_DIR}/SrcProp\"")
else()
  set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS
    "-ISrcProp")
endif()

add_executable(IncludeDirectories main.cpp)

if(USE_FULLPATH)
  set_target_properties(IncludeDirectories
    PROPERTIES COMPILE_FLAGS "\"-I${CMAKE_BINARY_DIR}/TarProp\"")
else()
  set_target_properties(IncludeDirectories
    PROPERTIES COMPILE_FLAGS "-ITarProp")
endif()

# Test escaping of special characters in include directory paths.
set(special_chars "~@&{}()!'")
if(NOT CMAKE_GENERATOR MATCHES "(Unix|MinGW|MSYS) Makefiles")
  # when compiler is used for dependencies, special characters for make are not escaped
  string(APPEND special_chars "%")
endif()
if(NOT CMAKE_GENERATOR STREQUAL "Watcom WMake")
  # Watcom seems to have no way to encode these characters.
  string(APPEND special_chars "#=[]")
endif()
if(NOT (MINGW AND CMAKE_GENERATOR MATCHES "(Unix|MSYS) Makefiles"))
  # FIXME: Dependencies work but command-line generation does not handle '$'.
  string(APPEND special_chars "$")
endif()
if(NOT CMAKE_GENERATOR MATCHES "(Borland|NMake) Makefiles")
  # NMake and Borland seem to have no way to encode a single '^'.
  string(APPEND special_chars "^")
endif()
if(NOT CMAKE_GENERATOR MATCHES "Visual Studio 9 2008|Watcom WMake")
  # The vcproj format separates values with ','.
  string(APPEND special_chars ",")
endif()
if(NOT WIN32 AND NOT CYGWIN)
  string(APPEND special_chars "*?<>")
endif()
set(special_dir "${CMAKE_CURRENT_BINARY_DIR}/special-${special_chars}-include")
file(WRITE "${special_dir}/SpecialDir.h" "#define SPECIAL_DIR_H\n")
target_include_directories(IncludeDirectories PRIVATE "${special_dir}")
target_compile_definitions(IncludeDirectories PRIVATE INCLUDE_SPECIAL_DIR)

if(MAKE_SUPPORTS_SPACES)
  set(special_space_dir "${CMAKE_CURRENT_BINARY_DIR}/special-space ${special_chars}-include")
  file(WRITE "${special_space_dir}/SpecialSpaceDir.h" "#define SPECIAL_SPACE_DIR_H\n")
  target_include_directories(IncludeDirectories PRIVATE "${special_space_dir}")
  target_compile_definitions(IncludeDirectories PRIVATE INCLUDE_SPECIAL_SPACE_DIR)
endif()

add_library(ordertest ordertest.cpp)
target_include_directories(ordertest SYSTEM PUBLIC SystemIncludeDirectories/systemlib)
target_include_directories(ordertest PUBLIC SystemIncludeDirectories/userlib)

# Test "IMPORTED_NO_SYSTEM" property and its interaction with "SYSTEM"
add_library(ordertest2 ordertest.cpp)
target_include_directories(ordertest2 SYSTEM PRIVATE SystemIncludeDirectories/systemlib)
target_link_libraries(ordertest2 PRIVATE ordertest2_userlib)
add_library(ordertest2_userlib INTERFACE IMPORTED)
target_include_directories(ordertest2_userlib INTERFACE SystemIncludeDirectories/userlib)
set_property(TARGET ordertest2_userlib PROPERTY IMPORTED_NO_SYSTEM 1)
get_property(system_prop_value TARGET ordertest2_userlib PROPERTY SYSTEM)
if (NOT system_prop_value)
  message(SEND_ERROR "ordertest2_userlib SYSTEM property should be ON.")
endif()

# Test "SYSTEM" property of non-imported libraries
add_library(ordertest3_systemlib INTERFACE)
target_include_directories(ordertest3_systemlib INTERFACE SystemIncludeDirectories/systemlib)
set_property(TARGET ordertest3_systemlib PROPERTY SYSTEM 1)
add_library(ordertest3_userlib INTERFACE)
target_include_directories(ordertest3_userlib INTERFACE SystemIncludeDirectories/userlib)
add_library(ordertest3 ordertest.cpp)
target_link_libraries(ordertest3 PRIVATE ordertest3_systemlib ordertest3_userlib)

# Test "SYSTEM" property of imported libraries and its interaction with "IMPORTED_NO_SYSTEM"
add_library(ordertest4 ordertest.cpp)
target_include_directories(ordertest4 SYSTEM PRIVATE SystemIncludeDirectories/systemlib)
target_link_libraries(ordertest4 PRIVATE ordertest4_userlib)
add_library(ordertest4_userlib INTERFACE IMPORTED)
target_include_directories(ordertest4_userlib INTERFACE SystemIncludeDirectories/userlib)
set_property(TARGET ordertest4_userlib PROPERTY SYSTEM 0)
get_property(imported_no_system_prop_value TARGET ordertest4_userlib PROPERTY IMPORTED_NO_SYSTEM)
if (imported_no_system_prop_value)
  message(SEND_ERROR "ordertest4_userlib IMPORTED_NO_SYSTEM property should be OFF.")
endif()

# Test the interaction between "SYSTEM" and "INTERFACE_SYSTEM_INCLUDE_DIRECTORIES"
add_library(ordertest5_systemlib INTERFACE)
target_include_directories(ordertest5_systemlib SYSTEM INTERFACE SystemIncludeDirectories/systemlib)
# The default value of `SYSTEM` is already `OFF`. Here we explicitly set it again.
set_property(TARGET ordertest5_systemlib PROPERTY SYSTEM 0)
add_library(ordertest5_userlib INTERFACE)
target_include_directories(ordertest5_userlib INTERFACE SystemIncludeDirectories/userlib)
add_library(ordertest5 ordertest.cpp)
target_link_libraries(ordertest5 PRIVATE ordertest5_systemlib ordertest5_userlib)

# Test that the include of imported executable is treated as system by default.
add_executable(ordertest6_systemexe IMPORTED)
target_include_directories(ordertest6_systemexe INTERFACE SystemIncludeDirectories/systemlib)
set_property(TARGET ordertest6_systemexe PROPERTY ENABLE_EXPORTS 1)
add_library(ordertest6_userlib INTERFACE)
target_include_directories(ordertest6_userlib INTERFACE SystemIncludeDirectories/userlib)
add_library(ordertest6 ordertest.cpp)
target_link_libraries(ordertest6 PRIVATE ordertest6_systemexe ordertest6_userlib)

# Test that the include of imported executable is not treated as system if "SYSTEM" property is OFF.
add_library(ordertest7 ordertest.cpp)
target_include_directories(ordertest7 SYSTEM PRIVATE SystemIncludeDirectories/systemlib)
target_link_libraries(ordertest7 PRIVATE ordertest7_userexe)
add_library(ordertest7_userexe INTERFACE IMPORTED)
target_include_directories(ordertest7_userexe INTERFACE SystemIncludeDirectories/userlib)
set_property(TARGET ordertest7_userexe PROPERTY ENABLE_EXPORTS 1)
set_property(TARGET ordertest7_userexe PROPERTY SYSTEM 0)

add_subdirectory(StandardIncludeDirectories)
add_subdirectory(TargetIncludeDirectories)

set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}")
get_property(propContent DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
if (NOT propContent STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "Setting DIRECTORY property failed.")
endif()
set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
get_property(propContentAfter DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
if (NOT propContentAfter STREQUAL "")
  message(SEND_ERROR "Clearing DIRECTORY property failed.")
endif()

add_library(empty_entry_test SHARED empty.cpp)
set_target_properties(empty_entry_test PROPERTIES INCLUDE_DIRECTORIES "")
include_directories(/one/two
  " "
  "  "
)
get_target_property(incs empty_entry_test INCLUDE_DIRECTORIES)
if (NOT incs STREQUAL ";/one/two")
  message(SEND_ERROR "Empty include_directories entry was not ignored.")
endif()

if(NOT CMAKE_GENERATOR STREQUAL "Xcode" AND NOT CMAKE_GENERATOR MATCHES "Ninja")
  add_subdirectory(CMP0021)
endif()
