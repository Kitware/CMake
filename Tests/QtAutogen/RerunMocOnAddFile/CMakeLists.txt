# This test checks whether adding a source file to the project triggers an AUTOMOC re-run.

cmake_minimum_required(VERSION 3.10)
project(RerunMocOnAddFile)
include("../AutogenCoreTest.cmake")

# Create an executable to generate a clean target
set(main_source "${CMAKE_CURRENT_BINARY_DIR}/generated_main.cpp")
file(WRITE "${main_source}" "int main() {}")
add_executable(exe "${main_source}")

# Utility variables
set(timeformat "%Y.%j.%H.%M%S")
set(testProjectTemplateDir "${CMAKE_CURRENT_SOURCE_DIR}/MocOnAddFile")
set(testProjectSrc "${CMAKE_CURRENT_BINARY_DIR}/MocOnAddFile")
set(testProjectBinDir "${CMAKE_CURRENT_BINARY_DIR}/MocOnAddFile-build")

# Utility macros
macro(sleep)
  message(STATUS "Sleeping for a few seconds.")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
endmacro()

macro(acquire_timestamp When)
  file(TIMESTAMP "${mocBasicBin}" time${When} "${timeformat}")
endmacro()

macro(rebuild buildName)
  message(STATUS "Starting build ${buildName}.")
  execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${testProjectBinDir}" RESULT_VARIABLE result)
  if (result)
    message(FATAL_ERROR "Build ${buildName} failed.")
  else()
    message(STATUS "Build ${buildName} finished.")
  endif()
endmacro()

macro(require_change)
  if (timeAfter VERSION_GREATER timeBefore)
    message(STATUS "As expected the file ${mocBasicBin} changed.")
  else()
    message(SEND_ERROR "Unexpectedly the file ${mocBasicBin} did not change!\nTimestamp pre: ${timeBefore}\nTimestamp aft: ${timeAfter}\n")
  endif()
endmacro()

macro(require_change_not)
  if (timeAfter VERSION_GREATER timeBefore)
    message(SEND_ERROR "Unexpectedly the file ${mocBasicBin} changed!\nTimestamp pre: ${timeBefore}\nTimestamp aft: ${timeAfter}\n")
  else()
    message(STATUS "As expected the file ${mocBasicBin} did not change.")
  endif()
endmacro()

# Create the test project from the template
unset(additional_project_sources)
unset(main_cpp_includes)
configure_file("${testProjectTemplateDir}/CMakeLists.txt.in" "${testProjectSrc}/CMakeLists.txt")
configure_file("${testProjectTemplateDir}/main.cpp.in" "${testProjectSrc}/main.cpp")

# Initial build
try_compile(MOC_RERUN
  "${testProjectBinDir}"
  "${testProjectSrc}"
  MocOnAddFile
  CMAKE_FLAGS "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_AUTOGEN_VERBOSE=${CMAKE_AUTOGEN_VERBOSE}"
              "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
  OUTPUT_VARIABLE output
)
if (NOT MOC_RERUN)
  message(FATAL_ERROR "Initial build of mocOnAddFile failed. Output: ${output}")
endif()

# Sleep to ensure new timestamps
sleep()

# Add a QObject class (defined in header) to the project and build
set(additional_project_sources myobject.cpp)
set(main_cpp_includes "#include \"myobject.h\"")
configure_file("${testProjectTemplateDir}/CMakeLists.txt.in" "${testProjectSrc}/CMakeLists.txt"
    @ONLY)
configure_file("${testProjectTemplateDir}/main.cpp.in" "${testProjectSrc}/main.cpp" @ONLY)
configure_file("${testProjectTemplateDir}/myobject.h" "${testProjectSrc}/myobject.h" COPYONLY)
configure_file("${testProjectTemplateDir}/myobject.cpp" "${testProjectSrc}/myobject.cpp" COPYONLY)
rebuild(2)

# Sleep to ensure new timestamps
sleep()

# Add a QObject class (defined in source) to the project and build
set(additional_project_sources myobject.cpp anotherobject.cpp)
configure_file("${testProjectTemplateDir}/CMakeLists.txt.in" "${testProjectSrc}/CMakeLists.txt"
    @ONLY)
configure_file("${testProjectTemplateDir}/anotherobject.cpp" "${testProjectSrc}/anotherobject.cpp"
    COPYONLY)
rebuild(3)
