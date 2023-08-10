include(RunCMake)

# Function to build and install a project.  The latter step *-check.cmake
# scripts can check installed files using the check_installed function.
function(run_install_test case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  run_cmake(${case})
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --config Debug)
  # Check "all" components.
  set(CMAKE_INSTALL_PREFIX ${RunCMake_TEST_BINARY_DIR}/root-all)
  run_cmake_command(${case}-all ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DBUILD_TYPE=Debug -P cmake_install.cmake)

  if(run_install_test_components)
    # Check unspecified component.
    set(CMAKE_INSTALL_PREFIX ${RunCMake_TEST_BINARY_DIR}/root-uns)
    run_cmake_command(${case}-uns ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DBUILD_TYPE=Debug -DCOMPONENT=Unspecified -P cmake_install.cmake)
    # Check explicit component.
    set(CMAKE_INSTALL_PREFIX ${RunCMake_TEST_BINARY_DIR}/root-exc)
    run_cmake_command(${case}-exc ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DBUILD_TYPE=Debug -DCOMPONENT=exc -P cmake_install.cmake)
    set(CMAKE_INSTALL_PREFIX ${RunCMake_TEST_BINARY_DIR}/root-lib)
    run_cmake_command(${case}-lib ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DBUILD_TYPE=Debug -DCOMPONENT=lib -P cmake_install.cmake)
    set(CMAKE_INSTALL_PREFIX ${RunCMake_TEST_BINARY_DIR}/root-dev)
    run_cmake_command(${case}-dev ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DBUILD_TYPE=Debug -DCOMPONENT=dev -P cmake_install.cmake)
  endif()
endfunction()

# Function called in *-check.cmake scripts to check installed files.
function(check_installed expect)
  file(GLOB_RECURSE actual
    LIST_DIRECTORIES TRUE
    RELATIVE ${CMAKE_INSTALL_PREFIX}
    ${CMAKE_INSTALL_PREFIX}/*
    )
  if(actual)
    list(SORT actual)
  endif()
  if(NOT "${actual}" MATCHES "${expect}")
    set(RunCMake_TEST_FAILED "Installed files:
  ${actual}
do not match what we expected:
  ${expect}
in directory:
  ${CMAKE_INSTALL_PREFIX}\n" PARENT_SCOPE)
  endif()
endfunction()

# Wrapper for run_cmake() that skips platforms on which we do not support editing the RPATH.
function(run_cmake_EDIT_RPATH_only case)
  if(UNIX AND CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG AND CMAKE_EXECUTABLE_FORMAT MATCHES "^(ELF|XCOFF)$")
    run_cmake(${case})
  else()
    # Sanity check against a platform known to be ELF-based
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      message(FATAL_ERROR "Expected platform Linux to advertise itself as ELF-based, but it did not.")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "AIX")
      message(FATAL_ERROR "Expected platform AIX to advertise itself as XCOFF-based, but it did not.")
    else()
      message(STATUS "${case} - SKIPPED (No ELF-based platform found)")
    endif()
  endif()
endfunction()

run_cmake(TARGETS-FILE_RPATH_CHANGE-old_rpath)
run_cmake_EDIT_RPATH_only(TARGETS-FILE_RPATH_CHANGE-new_rpath)
run_cmake(DIRECTORY-MESSAGE_NEVER)
run_cmake(DIRECTORY-PATTERN-MESSAGE_NEVER)
run_cmake(DIRECTORY-message)
run_cmake(DIRECTORY-message-lazy)
run_cmake(SkipInstallRulesWarning)
run_cmake(SkipInstallRulesNoWarning1)
run_cmake(SkipInstallRulesNoWarning2)
run_cmake(DIRECTORY-DIRECTORY-bad)
run_cmake(DIRECTORY-DESTINATION-bad)
run_cmake(FILES-DESTINATION-bad)
run_cmake(FILES-RENAME-bad)
run_cmake(TARGETS-DESTINATION-bad)
run_cmake(EXPORT-OldIFace)
run_cmake(EXPORT-UnknownExport)
run_cmake(EXPORT-NamelinkOnly)
run_cmake(EXPORT-SeparateNamelink)
run_cmake(EXPORT-TargetTwice)
run_cmake(CMP0062-OLD)
run_cmake(CMP0062-NEW)
run_cmake(CMP0062-WARN)
run_cmake(CMP0087-OLD)
run_cmake(CMP0087-NEW)
run_cmake(CMP0087-WARN)
run_cmake(TARGETS-ImportedGlobal)
run_cmake(TARGETS-NAMELINK_COMPONENT-bad-all)
run_cmake(TARGETS-NAMELINK_COMPONENT-bad-exc)
run_cmake(FILES-DESTINATION-TYPE)
run_cmake(DIRECTORY-DESTINATION-TYPE)
run_cmake(FILES-directory)
if(NOT WIN32)
    run_cmake(FILES-symlink-to-directory)
endif()

set(RunCMake_TEST_OPTIONS "-DCMAKE_BUILD_TYPE:STRING=Debug")
run_install_test(FILES-RENAME)
unset(RunCMake_TEST_OPTIONS)

if(APPLE)
  run_cmake(TARGETS-Apple-Defaults)
endif()

if(NOT RunCMake_GENERATOR STREQUAL "Xcode" OR NOT "$ENV{CMAKE_OSX_ARCHITECTURES}" MATCHES "[;$]")
  run_install_test(FILES-TARGET_OBJECTS)
endif()

if(CMake_TEST_ISPC)
  run_install_test(FILES-EXTRA_ISPC_TARGET_OBJECTS)
endif()


run_install_test(TARGETS-InstallFromSubDir)
run_install_test(TARGETS-OPTIONAL)
run_install_test(FILES-OPTIONAL)
run_install_test(DIRECTORY-OPTIONAL)
run_install_test(TARGETS-Defaults)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  run_install_test(TARGETS-NAMELINK-No-Tweak)
endif()

set(RunCMake_TEST_OPTIONS
  "-DCMAKE_INSTALL_BINDIR:PATH=mybin"
  "-DCMAKE_INSTALL_LIBDIR:PATH=mylib"
  "-DCMAKE_INSTALL_INCLUDEDIR:PATH=myinclude"
  )
run_install_test(TARGETS-Defaults-Cache)
unset(RunCMake_TEST_OPTIONS)

run_install_test(FILES-TYPE)
run_install_test(DIRECTORY-TYPE)

set(RunCMake_TEST_OPTIONS
  "-DCMAKE_INSTALL_BINDIR:PATH=mybin"
  "-DCMAKE_INSTALL_SBINDIR:PATH=mysbin"
  "-DCMAKE_INSTALL_LIBEXECDIR:PATH=mylibexec"
  "-DCMAKE_INSTALL_LIBDIR:PATH=mylib"
  "-DCMAKE_INSTALL_INCLUDEDIR:PATH=myinclude"
  "-DCMAKE_INSTALL_SYSCONFDIR:PATH=myetc"
  "-DCMAKE_INSTALL_SHAREDSTATEDIR:PATH=mycom"
  "-DCMAKE_INSTALL_LOCALSTATEDIR:PATH=myvar"
  "-DCMAKE_INSTALL_RUNSTATEDIR:PATH=myrun"
  "-DCMAKE_INSTALL_DATADIR:PATH=myshare"
  "-DCMAKE_INSTALL_INFODIR:PATH=myinfo"
  "-DCMAKE_INSTALL_LOCALEDIR:PATH=mylocale"
  "-DCMAKE_INSTALL_MANDIR:PATH=myman"
  "-DCMAKE_INSTALL_DOCDIR:PATH=mydoc"
  )
run_install_test(FILES-TYPE-Cache)
run_install_test(DIRECTORY-TYPE-Cache)
unset(RunCMake_TEST_OPTIONS)

set(RunCMake_TEST_OPTIONS
  "-DCMAKE_INSTALL_LOCALSTATEDIR:PATH=myvar"
  "-DCMAKE_INSTALL_DATAROOTDIR:PATH=myshare"
  )
run_install_test(FILES-TYPE-CacheDependent)
run_install_test(DIRECTORY-TYPE-CacheDependent)
unset(RunCMake_TEST_OPTIONS)

set(RunCMake_TEST_OPTIONS "-DCMAKE_BUILD_TYPE:STRING=Debug")
run_install_test(TARGETS-OUTPUT_NAME)
unset(RunCMake_TEST_OPTIONS)

run_install_test(Deprecated)
run_install_test(PRE_POST_INSTALL_SCRIPT)
run_install_test(TARGETS-CONFIGURATIONS)
run_install_test(DIRECTORY-PATTERN)
run_install_test(TARGETS-Parts)
run_install_test(FILES-PERMISSIONS)
run_install_test(TARGETS-RPATH)
run_install_test(InstallRequiredSystemLibraries)

set(RunCMake_TEST_OPTIONS "-DCMAKE_POLICY_DEFAULT_CMP0087:STRING=NEW")
run_install_test(SCRIPT)
unset(RunCMake_TEST_OPTIONS)

if(UNIX)
  run_install_test(DIRECTORY-symlink-clobber)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  run_cmake(TARGETS-RUNTIME_DEPENDENCIES-macos-two-bundle)
  run_cmake(TARGETS-RUNTIME_DEPENDENCIES-macos-no-framework)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "^(Linux|Darwin|Windows)$")
  run_install_test(TARGETS-RUNTIME_DEPENDENCIES-nodep)
  run_install_test(TARGETS-RUNTIME_DEPENDENCIES-empty)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}")
  run_cmake(TARGETS-RUNTIME_DEPENDENCIES-cross)
  unset(RunCMake_TEST_OPTIONS)
  run_cmake(TARGETS-RUNTIME_DEPENDENCY_SET-RUNTIME_DEPENDENCIES-conflict)
  run_cmake(RuntimeDependencies-COMPONENTS)
else()
  run_cmake(TARGETS-RUNTIME_DEPENDENCIES-unsupported)
  run_cmake(TARGETS-RUNTIME_DEPENDENCY_SET-unsupported)
  run_cmake(IMPORTED_RUNTIME_ARTIFACTS-RUNTIME_DEPENDENCY_SET-unsupported)
  run_cmake(RUNTIME_DEPENDENCY_SET-unsupported)
endif()

set(run_install_test_components 1)
run_install_test(FILES-EXCLUDE_FROM_ALL)
run_install_test(TARGETS-EXCLUDE_FROM_ALL)
run_install_test(TARGETS-NAMELINK_COMPONENT)
run_install_test(SCRIPT-COMPONENT)
run_install_test(SCRIPT-ALL_COMPONENTS)
