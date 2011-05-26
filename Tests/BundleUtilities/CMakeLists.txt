cmake_minimum_required(VERSION 2.8)
project(BundleUtilities)

###### the various types of dependencies we can have

# a shared library
add_library(shared SHARED shared.cpp shared.h)

# another shared library
add_library(shared2 SHARED shared2.cpp shared2.h)

# a loadable module (depends on shared2)
# test app will load this at runtime
add_library(module MODULE module.cpp module.h)
set_target_properties(module PROPERTIES PREFIX "")
get_target_property(module_loc module LOCATION)
target_link_libraries(module shared2)

# a framework library
add_library(framework SHARED framework.cpp framework.h)
# TODO: fix problems with local frameworks without rpaths
#set_target_properties(framework PROPERTIES FRAMEWORK 1)

# make sure rpaths are not helping BundleUtilities or the executables
set_target_properties(shared shared2 module framework PROPERTIES
                      SKIP_BUILD_RPATH 1)


######  test a Bundle application using dependencies

set(TESTBUNDLEDIR "${CMAKE_CURRENT_BINARY_DIR}/testdir1")
add_executable(testbundleutils1 MACOSX_BUNDLE testbundleutils.cpp)
target_link_libraries(testbundleutils1 shared framework ${CMAKE_DL_LIBS})
set_target_properties(testbundleutils1 PROPERTIES
                      INSTALL_RPATH "${TESTBUNDLEDIR}"
                      BUILD_WITH_INSTALL_RPATH 1)
get_target_property(loc testbundleutils1 LOCATION)

add_custom_target(testbundleutils1_test  ALL
  COMMAND ${CMAKE_COMMAND}
  "-DINPUT=${loc}"
  "-DMODULE=${module_loc}"
  "-DINPUTDIR=${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}"
  "-DOUTPUTDIR=${TESTBUNDLEDIR}"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/bundleutils.cmake"
  )

add_dependencies(testbundleutils1_test testbundleutils1)



######  test a non-Bundle application using dependencies

set(TESTBUNDLEDIR "${CMAKE_CURRENT_BINARY_DIR}/testdir2")
add_executable(testbundleutils2 testbundleutils.cpp)
target_link_libraries(testbundleutils2 shared framework ${CMAKE_DL_LIBS})
set_target_properties(testbundleutils2 PROPERTIES
                      INSTALL_RPATH "${TESTBUNDLEDIR}"
                      BUILD_WITH_INSTALL_RPATH 1)
get_target_property(loc testbundleutils2 LOCATION)

add_custom_target(testbundleutils2_test  ALL
  COMMAND ${CMAKE_COMMAND}
  "-DINPUT=${loc}"
  "-DMODULE=${module_loc}"
  "-DINPUTDIR=${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}"
  "-DOUTPUTDIR=${TESTBUNDLEDIR}"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/bundleutils.cmake"
  )
add_dependencies(testbundleutils2_test testbundleutils2)
