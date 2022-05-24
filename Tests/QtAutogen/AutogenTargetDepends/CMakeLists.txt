cmake_minimum_required(VERSION 3.16)
project(AutogenTargetDepends)
include("../AutogenCoreTest.cmake")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
set(CSD ${CMAKE_CURRENT_SOURCE_DIR})
set(CBD ${CMAKE_CURRENT_BINARY_DIR})

# -- Test AUTOGEN_TARGET_DEPENDS with GENERATED file dependency
#
# This tests the dependency of the mocDepATDFile_autogen target of
# mocDepATDTarget to the utility target mocDepATDFileUtil.
# If mocDepATDFile_autogen gets built *before* or in *parallel* to
# mocDepATDFileUtil, the build will fail. That's
# because ATDFile.hpp, which is required by mocDepATDFile_autogen,
# is only valid after the mocDepATDFileUtil build has been completed.
#
# The sleep seconds artificially increase the build time of
# mocDepATDFileUtil to simulate a slow utility target build that takes
# longer to run than the build of the mocDepATDFile_autogen target.
add_custom_command(
  OUTPUT ${CBD}/ATDFile.hpp
  COMMAND ${CMAKE_COMMAND} -E copy ${CSD}/object_invalid.hpp.in ${CBD}/ATDFile.hpp
  COMMAND ${CMAKE_COMMAND} -E sleep 3
  COMMAND ${CMAKE_COMMAND} -E copy ${CSD}/object_valid.hpp.in ${CBD}/ATDFile.hpp)

add_executable(mocDepATDFile testATDFile.cpp)
target_link_libraries(mocDepATDFile ${QT_QTCORE_TARGET})
set_target_properties(mocDepATDFile PROPERTIES AUTOMOC TRUE)
set_target_properties(mocDepATDFile PROPERTIES AUTOGEN_TARGET_DEPENDS ${CBD}/ATDFile.hpp)


# -- Test AUTOGEN_TARGET_DEPENDS with target dependency
#
# This tests the dependency of the mocDepATDTarget_autogen target of
# mocDepATDTarget to the utility target mocDepATDTargetUtil.
# If mocDepATDTarget_autogen gets built *before* or in *parallel* to
# mocDepATDTargetUtil, the build will fail. That's
# because ATDTarget.hpp, which is required by mocDepATDTarget_autogen,
# is only valid after the mocDepATDTargetUtil build has been completed.
#
# The sleep seconds artificially increase the build time of
# mocDepATDTargetUtil to simulate a slow utility target build that takes
# longer to run than the build of the mocDepATDTarget_autogen target.
add_custom_target(mocDepATDTargetUtil
  BYPRODUCTS ${CBD}/ATDTarget.hpp
  COMMAND ${CMAKE_COMMAND} -E copy ${CSD}/object_invalid.hpp.in ${CBD}/ATDTarget.hpp
  COMMAND ${CMAKE_COMMAND} -E sleep 3
  COMMAND ${CMAKE_COMMAND} -E copy ${CSD}/object_valid.hpp.in ${CBD}/ATDTarget.hpp)

add_executable(mocDepATDTarget testATDTarget.cpp)
target_link_libraries(mocDepATDTarget ${QT_QTCORE_TARGET})
set_target_properties(mocDepATDTarget PROPERTIES AUTOMOC TRUE)
set_target_properties(mocDepATDTarget PROPERTIES AUTOGEN_TARGET_DEPENDS mocDepATDTargetUtil)
