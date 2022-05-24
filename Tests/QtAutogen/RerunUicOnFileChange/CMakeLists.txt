cmake_minimum_required(VERSION 3.17)
project(RerunUicOnFileChange)
include("../AutogenGuiTest.cmake")

# Utility variables
set(testProjectTemplateDir "${CMAKE_CURRENT_SOURCE_DIR}/UicOnFileChange")
set(testProjectSrc "${CMAKE_CURRENT_BINARY_DIR}/UicOnFileChange")
set(testProjectBinDir "${CMAKE_CURRENT_BINARY_DIR}/UicOnFileChange-build")

set(TEST_CONFIG "Release")

macro(sleep)
  message(STATUS "Sleeping for a few seconds.")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
endmacro()
macro(rebuild buildName)
  message(STATUS "Starting build ${buildName}.")
  execute_process(COMMAND "${CMAKE_COMMAND}" --build . --config "${TEST_CONFIG}"
    WORKING_DIRECTORY "${testProjectBinDir}" RESULT_VARIABLE result
  )
  if (result)
    message(FATAL_ERROR "Build ${buildName} failed.")
  else()
    message(STATUS "Build ${buildName} finished.")
  endif()
endmacro()

configure_file("${testProjectTemplateDir}/mocwidget.h" "${testProjectSrc}/mocwidget.h" COPYONLY)
configure_file("${testProjectTemplateDir}/mainwindow.h" "${testProjectSrc}/mainwindow.h" COPYONLY)
configure_file("${testProjectTemplateDir}/main.cpp" "${testProjectSrc}/main.cpp" COPYONLY)
configure_file("${testProjectTemplateDir}/CMakeLists.txt.in" "${testProjectSrc}/CMakeLists.txt" @ONLY)

set(Num 1)
configure_file("${testProjectTemplateDir}/mainwindow.ui.in" "${testProjectSrc}/mainwindow.ui" @ONLY)

if(CMAKE_GENERATOR_INSTANCE)
    set(_D_CMAKE_GENERATOR_INSTANCE "-DCMAKE_GENERATOR_INSTANCE=${CMAKE_GENERATOR_INSTANCE}")
else()
    set(_D_CMAKE_GENERATOR_INSTANCE "")
endif()

get_property(is_multi GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(is_multi)
  set(build_type_extra "-DCMAKE_CONFIGURATION_TYPES=${TEST_CONFIG}")
  set(extra_bin_path "${TEST_CONFIG}/")
else()
  set(build_type_extra "-DCMAKE_BUILD_TYPE=${TEST_CONFIG}")
endif()

# Set the environment PATH/LD_LIBRARY_PATH variables to run the resulting executable
if(WIN32 AND TARGET ${QT_QTCORE_TARGET})
  get_target_property(qtcore_path ${QT_QTCORE_TARGET} LOCATION)
  if(NOT qtcore_path)
    get_target_property(qtcore_path ${QT_QTCORE_TARGET} IMPORTED_LOCATION)
  endif()
  get_filename_component(qtcore_path "${qtcore_path}" DIRECTORY)
  set(ENV{PATH} "${qtcore_path};$ENV{PATH}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -B "${testProjectBinDir}" -S "${testProjectSrc}"
          -G "${CMAKE_GENERATOR}"
          -A "${CMAKE_GENERATOR_PLATFORM}"
          -T "${CMAKE_GENERATOR_TOOLSET}"
          ${_D_CMAKE_GENERATOR_INSTANCE}
          "${build_type_extra}"
          "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
          "-DCMAKE_AUTOGEN_VERBOSE=${CMAKE_AUTOGEN_VERBOSE}"
          "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
          "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
  RESULT_VARIABLE exit_code
  OUTPUT_VARIABLE output
  ERROR_VARIABLE output
)
if(NOT exit_code EQUAL 0)
  message(FATAL_ERROR "Initial configuration of UicOnFileChange failed. Output: ${output}")
endif()

# Initial build
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${testProjectBinDir}" --config "${TEST_CONFIG}"
    RESULT_VARIABLE exit_code
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
)
if(NOT exit_code EQUAL 0)
    message(FATAL_ERROR "Initial build of UicOnFileChange failed. Output: ${output}")
endif()

execute_process(COMMAND "${testProjectBinDir}/${extra_bin_path}UicOnFileChange" RESULT_VARIABLE result)
if(NOT result EQUAL "1")
  message(FATAL_ERROR "Initial build of UicOnFileChange test result is: ${result}")
endif()

sleep()

set(Num 2)
configure_file("${testProjectTemplateDir}/mainwindow.ui.in" "${testProjectSrc}/mainwindow.ui" @ONLY)
rebuild(2)

execute_process(COMMAND "${testProjectBinDir}/${extra_bin_path}UicOnFileChange" RESULT_VARIABLE result)
if(NOT result EQUAL "0")
  message(FATAL_ERROR "Rebuild of UicOnFileChange test result is: ${result}")
endif()

# Check if the generated ui_mainwindow.h rules introduce circular dependency between the generated
# ui_mainwinow.h and timestamp.
#
# The first rebuild updates a timestamp dependency file after "touching" mainwindow.h.
sleep()
execute_process(COMMAND ${CMAKE_COMMAND} -E touch "${testProjectSrc}/mainwindow.h")
rebuild(3)

# The second rebuild detects if cycling dependency is introduced by deps file.
sleep()
rebuild(4)
