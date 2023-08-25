include(${CMAKE_CURRENT_LIST_DIR}/cm_message_checks_compat.cmake)

function(cm_check_cxx_feature name)
  set(TRY_RUN_FEATURE "${ARGN}")
  string(TOUPPER ${name} FEATURE)
  if(NOT DEFINED CMake_HAVE_CXX_${FEATURE})
    cm_message_checks_compat(
      "Checking if compiler supports C++ ${name}"
      __checkStart __checkPass __checkFail)
    message(${__checkStart})
    if(CMAKE_CXX_STANDARD)
      set(maybe_cxx_standard -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})
    else()
      set(maybe_cxx_standard "")
    endif()
    if (TRY_RUN_FEATURE)
      try_run(CMake_RUN_CXX_${FEATURE} CMake_COMPILE_CXX_${FEATURE}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/cm_cxx_${name}.cxx
        LINK_LIBRARIES ${cm_check_cxx_feature_LINK_LIBRARIES}
        CMAKE_FLAGS ${maybe_cxx_standard}
        OUTPUT_VARIABLE OUTPUT
        )
      if (CMake_RUN_CXX_${FEATURE} EQUAL "0" AND CMake_COMPILE_CXX_${FEATURE})
        set(CMake_HAVE_CXX_${FEATURE} ON CACHE INTERNAL "TRY_RUN" FORCE)
      else()
        set(CMake_HAVE_CXX_${FEATURE} OFF CACHE INTERNAL "TRY_RUN" FORCE)
      endif()
    else()
      try_compile(CMake_HAVE_CXX_${FEATURE}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/cm_cxx_${name}.cxx
        LINK_LIBRARIES ${cm_check_cxx_feature_LINK_LIBRARIES}
        CMAKE_FLAGS ${maybe_cxx_standard}
        OUTPUT_VARIABLE OUTPUT
        )
    endif()
    set(check_output "${OUTPUT}")
    # Filter out MSBuild output that looks like a warning.
    string(REGEX REPLACE " +0 Warning\\(s\\)" "" check_output "${check_output}")
    # Filter out MSBuild output that looks like a warning.
    string(REGEX REPLACE "[^\n]*warning MSB[0-9][0-9][0-9][0-9][^\n]*" "" check_output "${check_output}")
    # Filter out MSVC output that looks like a command-line warning.
    string(REGEX REPLACE "[^\n]*warning D[0-9][0-9][0-9][0-9][^\n]*" "" check_output "${check_output}")
    # Filter out warnings caused by user flags.
    string(REGEX REPLACE "[^\n]*warning:[^\n]*-Winvalid-command-line-argument[^\n]*" "" check_output "${check_output}")
    # Filter out warnings caused by local configuration.
    string(REGEX REPLACE "[^\n]*warning:[^\n]*directory not found for option[^\n]*" "" check_output "${check_output}")
    string(REGEX REPLACE "[^\n]*warning:[^\n]*object file compiled with -mlong-branch which is no longer needed[^\n]*" "" check_output "${check_output}")
    # Filter out other warnings unrelated to feature checks.
    string(REGEX REPLACE "[^\n]*warning:[^\n]*sprintf\\(\\) is often misused, please use snprintf[^\n]*" "" check_output "${check_output}")
    # Filter out libhugetlbfs warnings.
    string(REGEX REPLACE "[^\n]*libhugetlbfs [^\n]*: WARNING[^\n]*" "" check_output "${check_output}")
    # Filter out xcodebuild warnings.
    string(REGEX REPLACE "[^\n]* xcodebuild\\[[0-9]*:[0-9]*\\][^\n]*[Ww]arning: [^\n]*" "" check_output "${check_output}")
    # Filter out icpc warnings
    string(REGEX REPLACE "[^\n]*icpc: command line warning #10121: overriding [^\n]*" "" check_output "${check_output}")
    # Filter out ld warnings.
    string(REGEX REPLACE "[^\n]*ld: warning: [^\n]*" "" check_output "${check_output}")
    # If using the feature causes warnings, treat it as broken/unavailable.
    if(check_output MATCHES "(^|[ :])[Ww][Aa][Rr][Nn][Ii][Nn][Gg]")
      set(CMake_HAVE_CXX_${FEATURE} OFF CACHE INTERNAL "TRY_COMPILE" FORCE)
    endif()
    if(CMake_HAVE_CXX_${FEATURE})
      message(${__checkPass} "yes")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if compiler supports C++ ${name} passed with the following output:\n"
        "${OUTPUT}\n"
        "\n"
        )
    else()
      message(${__checkFail} "no")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if compiler supports C++ ${name} failed with the following output:\n"
        "${OUTPUT}\n"
        "\n"
        )
    endif()
  endif()
endfunction()

cm_check_cxx_feature(make_unique)
if(CMake_HAVE_CXX_MAKE_UNIQUE)
  set(CMake_HAVE_CXX_UNIQUE_PTR 1)
endif()
cm_check_cxx_feature(unique_ptr)
if (NOT CMAKE_CXX_STANDARD LESS "17")
  if (NOT CMAKE_CROSSCOMPILING OR CMAKE_CROSSCOMPILING_EMULATOR)
    cm_check_cxx_feature(filesystem TRY_RUN)
  else()
    # In cross-compiling mode, it is not possible to check implementation bugs
    # so rely only on conformance done by compilation
    cm_check_cxx_feature(filesystem)
  endif()
else()
  set(CMake_HAVE_CXX_FILESYSTEM FALSE)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|aarch64)$")
  cm_check_cxx_feature(atomic_builtin)
  if(NOT CMake_HAVE_CXX_ATOMIC_BUILTIN)
    set(cm_check_cxx_feature_LINK_LIBRARIES atomic)
    cm_check_cxx_feature(atomic_lib) # defines CMake_HAVE_CXX_ATOMIC_LIB
    unset(cm_check_cxx_feature_LINK_LIBRARIES)
  endif()
endif()
