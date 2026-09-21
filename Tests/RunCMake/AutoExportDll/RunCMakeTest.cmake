include(RunCMake)
set(RunCMake_TEST_NO_CLEAN TRUE)

function (run_cmake_AutoExport name dir)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${name}-build")
  # start by cleaning up because we don't clean up along the way
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  # configure the AutoExport test
  run_cmake(${name})
  unset(RunCMake_TEST_OPTIONS)
  # don't run this test on Watcom or Borland make as it is not supported
  if(RunCMake_GENERATOR MATCHES "Watcom WMake|Borland Makefiles|FASTBuild")
    return()
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "OrangeC")
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
  run_cmake_command(${name}Build ${CMAKE_COMMAND} --build
    ${RunCMake_TEST_BINARY_DIR} --config Debug --clean-first)
  # save the current timestamp of exports.def
  if(EXPORTS)
    set(EXPORTS_DEF "${RunCMake_TEST_BINARY_DIR}/${dir}/${INTDIR}exports.def")
    if(NOT EXISTS "${EXPORTS_DEF}")
      set(EXPORTS_DEF
        "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/${dir}/${INTDIR}exports.def")
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
  run_cmake_command(${name}Run
    ${RunCMake_TEST_BINARY_DIR}/bin/${INTDIR}say${EXE_EXT})
  # build AutoExport again without modification
  run_cmake_command(${name}BuildAgain ${CMAKE_COMMAND} --build
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
endfunction ()
run_cmake_AutoExport(AutoExport "say.dir")
if (RunCMake_GENERATOR MATCHES "(Ninja|Makefiles|Visual Studio)")
  run_cmake_AutoExport(AutoExportShort ".o/0cb3d702")
endif ()

# Verify that the two bindexplib backends, the COFF symbol table walker used
# on Windows hosts and the nm reader used on other hosts, export the same
# symbols from a fixed set of object files.
function(run_create_def)
  set(nm_arg "")
  if(NOT CMAKE_HOST_WIN32)
    # Reading COFF object files on other hosts requires llvm-nm.
    find_program(LLVM_NM NAMES llvm-nm)
    if(NOT LLVM_NM)
      return()
    endif()
    # llvm-nm 16 is the first version that reads ARM64EC object files.
    execute_process(COMMAND "${LLVM_NM}" --version
      OUTPUT_VARIABLE nm_version ERROR_QUIET)
    if(NOT nm_version MATCHES "LLVM version ([0-9]+)" OR CMAKE_MATCH_1 LESS 16)
      return()
    endif()
    set(nm_arg "--nm=${LLVM_NM}")
  endif()
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/CreateDef-build")
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake_command(CreateDef ${CMAKE_COMMAND}
    "-Dcmake=${CMAKE_COMMAND}"
    "-Dnm=${nm_arg}"
    "-Dsrc=${RunCMake_SOURCE_DIR}"
    "-Dbin=${RunCMake_TEST_BINARY_DIR}"
    -P "${RunCMake_SOURCE_DIR}/CreateDef.cmake"
    )
endfunction()
run_create_def()

function(run_AIXExportExplicit)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/AIXExportExplicit-build")
  run_cmake(AIXExportExplicit)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE TRUE)
  run_cmake_command(AIXExportExplicit-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()
if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
  run_AIXExportExplicit()
endif()
