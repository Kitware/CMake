include(RunCMake)
set(RunCMake_TEST_NO_CLEAN TRUE)
set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/AutoExport-build")
# start by cleaning up because we don't clean up along the way
file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
# configure the AutoExport test
run_cmake(AutoExport)
unset(RunCMake_TEST_OPTIONS)
# don't run this test on Watcom or Borland make as it is not supported
if(RunCMake_GENERATOR MATCHES "Watcom WMake|Borland Makefiles")
  return()
endif()
if(RunCMake_GENERATOR MATCHES "Ninja|Visual Studio" AND
   CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(EXPORTS TRUE)
endif()
# we build debug so the say.exe will be found in Debug/say.exe for
# Visual Studio generators
if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(INTDIR "Debug/")
endif()
# build AutoExport
run_cmake_command(AutoExportBuild ${CMAKE_COMMAND} --build
  ${RunCMake_TEST_BINARY_DIR} --config Debug --clean-first)
# save the current timestamp of exports.def
if(EXPORTS)
  set(EXPORTS_DEF "${RunCMake_TEST_BINARY_DIR}/say.dir/${INTDIR}exports.def")
  if(NOT EXISTS "${EXPORTS_DEF}")
    set(EXPORTS_DEF
      "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/say.dir/${INTDIR}exports.def")
  endif()
  file(TIMESTAMP "${EXPORTS_DEF}" timestamp)
  if(NOT timestamp)
    message(SEND_ERROR "Could not get timestamp for \"${EXPORTS_DEF}\"")
  endif()
endif()
# run the executable that uses symbols from the dll
if(WIN32)
  set(EXE_EXT ".exe")
endif()
run_cmake_command(AutoExportRun
  ${RunCMake_TEST_BINARY_DIR}/bin/${INTDIR}say${EXE_EXT})
# build AutoExport again without modification
run_cmake_command(AutoExportBuildAgain ${CMAKE_COMMAND} --build
  ${RunCMake_TEST_BINARY_DIR} --config Debug)
# compare timestamps of exports.def to make sure it has not been updated
if(EXPORTS)
  file(TIMESTAMP "${EXPORTS_DEF}" timestamp_after)
  if(NOT timestamp_after)
    message(SEND_ERROR "Could not get timestamp for \"${EXPORTS_DEF}\"")
  endif()
  if (timestamp_after STREQUAL timestamp)
    message(STATUS "AutoExportTimeStamp - PASSED")
  else()
    message(SEND_ERROR "\"${EXPORTS_DEF}\" has been updated.")
  endif()
endif()

function(run_AIXExportExplicit)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/AIXExpotExplicit-build")
  run_cmake(AIXExportExplicit)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  run_cmake_command(AIXExportExplicit-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()
if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
  run_AIXExportExplicit()
endif()
