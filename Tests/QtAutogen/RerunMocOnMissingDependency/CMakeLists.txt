# This test checks whether a missing dependency of the moc output triggers an AUTOMOC re-run.

cmake_minimum_required(VERSION 3.16)
project(RerunMocOnMissingDependency)
include("../AutogenCoreTest.cmake")

# Create an executable to generate a clean target
set(main_source "${CMAKE_CURRENT_BINARY_DIR}/generated_main.cpp")
file(WRITE "${main_source}" "int main() {}")
add_executable(exe "${main_source}")

# Utility variables
set(testProjectTemplateDir "${CMAKE_CURRENT_SOURCE_DIR}/MocOnMissingDependency")
set(testProjectSrc "${CMAKE_CURRENT_BINARY_DIR}/MocOnMissingDependency")
set(testProjectBinDir "${CMAKE_CURRENT_BINARY_DIR}/MocOnMissingDependency-build")
if(DEFINED Qt${QT_TEST_VERSION}Core_VERSION AND Qt${QT_TEST_VERSION}Core_VERSION VERSION_GREATER_EQUAL "5.15.0")
  set(moc_depfiles_supported TRUE)
else()
  set(moc_depfiles_supported FALSE)
endif()

# Utility macros
macro(sleep)
  message(STATUS "Sleeping for a few seconds.")
  execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
endmacro()

macro(rebuild buildName)
  message(STATUS "Starting build ${buildName}.")
  execute_process(COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY "${testProjectBinDir}"
      RESULT_VARIABLE result OUTPUT_VARIABLE output)
  if (result)
    message(FATAL_ERROR "Build ${buildName} failed.")
  else()
    message(STATUS "Build ${buildName} finished.")
  endif()
endmacro()

# Create the test project from the template
file(COPY "${testProjectTemplateDir}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
configure_file("${testProjectTemplateDir}/CMakeLists.txt.in" "${testProjectSrc}/CMakeLists.txt" @ONLY)

# Initial build
file(REMOVE_RECURSE "${testProjectBinDir}")
try_compile(MOC_RERUN
  "${testProjectBinDir}"
  "${testProjectSrc}"
  MocOnMissingDependency
  CMAKE_FLAGS "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_AUTOGEN_VERBOSE=ON"
              "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
              "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
  OUTPUT_VARIABLE output
)
if (NOT MOC_RERUN)
  message(FATAL_ERROR "Initial build of mocOnMissingDependency failed. Output: ${output}")
endif()

# Sleep to ensure new timestamps
sleep()

if(moc_depfiles_supported)
    # Remove the dependency inc1/foo.h and build again.
    # We expect that the moc_XXX.cpp file gets re-generated. But only if we have depfile support.
    file(REMOVE_RECURSE "${testProjectSrc}/inc1")
    rebuild(2)
    if(NOT output MATCHES "AutoMoc: Generating \"[^\"]*moc_myobject.cpp\"")
        message(FATAL_ERROR "moc_myobject.cpp was not re-generated "
            "after removing one of its dependencies")
    endif()
endif()

# Sleep to ensure new timestamps
sleep()

# The next build should *not* re-renerate any moc outputs
rebuild(3)
if(output MATCHES "AutoMoc: Generating")
    message(FATAL_ERROR "moc_myobject.cpp was not re-generated "
        "after removing one of its dependencies")
endif()
