cmake_minimum_required(VERSION 2.8)
project(BundleUtilities)

###### the various types of dependencies we can have

# a shared library
add_library(shared SHARED shared.cpp shared.h)

# another shared library
add_library(shared2 SHARED shared2.cpp shared2.h)


# a framework library
add_library(framework SHARED framework.cpp framework.h)
# TODO: fix problems with local frameworks without rpaths
#set_target_properties(framework PROPERTIES FRAMEWORK 1)

# make sure rpaths are not helping BundleUtilities or the executables
set_target_properties(shared shared2 framework PROPERTIES
                      SKIP_BUILD_RPATH 1)


######  test a Bundle application using dependencies

# a loadable module (depends on shared2)
# testbundleutils1 will load this at runtime
add_library(module1 MODULE module.cpp module.h)
set_target_properties(module1 PROPERTIES PREFIX "")
get_target_property(module_loc module1 LOCATION)
target_link_libraries(module1 shared2)

# a bundle application
add_executable(testbundleutils1 MACOSX_BUNDLE testbundleutils1.cpp)
target_link_libraries(testbundleutils1 shared framework ${CMAKE_DL_LIBS})
get_target_property(loc testbundleutils1 LOCATION)

set_target_properties(testbundleutils1 module1 PROPERTIES
                      INSTALL_RPATH "${CMAKE_CURRENT_BINARY_DIR}/testdir1"
                      BUILD_WITH_INSTALL_RPATH 1)

# add custom target to install and test the app
add_custom_target(testbundleutils1_test  ALL
  COMMAND ${CMAKE_COMMAND}
  "-DINPUT=${loc}"
  "-DMODULE=${module_loc}"
  "-DINPUTDIR=${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}"
  "-DOUTPUTDIR=${CMAKE_CURRENT_BINARY_DIR}/testdir1"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/bundleutils.cmake"
  DEPENDS testbundleutils1 module1
  )

add_dependencies(testbundleutils1_test testbundleutils1)



######  test a non-Bundle application using dependencies

# a loadable module (depends on shared2)
# testbundleutils2 will load this at runtime
add_library(module2 MODULE module.cpp module.h)
set_target_properties(module2 PROPERTIES PREFIX "")
get_target_property(module_loc module2 LOCATION)
target_link_libraries(module2 shared2)

# a non-bundle application
add_executable(testbundleutils2 testbundleutils2.cpp)
target_link_libraries(testbundleutils2 shared framework ${CMAKE_DL_LIBS})
get_target_property(loc testbundleutils2 LOCATION)

set_target_properties(testbundleutils2 module2 PROPERTIES
                      INSTALL_RPATH "${CMAKE_CURRENT_BINARY_DIR}/testdir2"
                      BUILD_WITH_INSTALL_RPATH 1)

# add custom target to install and test the app
add_custom_target(testbundleutils2_test  ALL
  COMMAND ${CMAKE_COMMAND}
  "-DINPUT=${loc}"
  "-DMODULE=${module_loc}"
  "-DINPUTDIR=${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}"
  "-DOUTPUTDIR=${CMAKE_CURRENT_BINARY_DIR}/testdir2"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/bundleutils.cmake"
  DEPENDS testbundleutils1 module2
  )
add_dependencies(testbundleutils2_test testbundleutils2)
