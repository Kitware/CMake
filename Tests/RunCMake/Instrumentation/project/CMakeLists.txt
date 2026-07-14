cmake_minimum_required(VERSION 4.3)
project(instrumentation C)
enable_testing()
if (EXISTS ${INSTRUMENT_COMMAND_FILE})
  include(${INSTRUMENT_COMMAND_FILE})
endif()

add_executable(main main.c)
add_library(lib lib.c)
target_link_libraries(main lib)

if (INSTRUMENT_COMPILE_TRACE STREQUAL "DEFAULT")
  target_compile_options(main PRIVATE "-ftime-trace")
  target_compile_options(lib PRIVATE "-ftime-trace")
elseif (INSTRUMENT_COMPILE_TRACE STREQUAL "EXPLICIT")
  target_compile_options(main PRIVATE "-ftime-trace=main.json")
  target_compile_options(lib PRIVATE "-ftime-trace=lib.json")
endif()

add_custom_command(TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E true
)
add_custom_command(
  COMMAND ${CMAKE_COMMAND} -E true
  OUTPUT output1 output2
)
add_custom_command(
  COMMAND $<TARGET_FILE:main>
  OUTPUT output3
  DEPENDS main
)

file(COPY_FILE shell_redirect.txt ${CMAKE_CURRENT_BINARY_DIR}/shell_redirect.in)
if(CMAKE_GENERATOR STREQUAL "Watcom WMake")
  # Watcom WMake does not seem to understand shell redirection.
  add_custom_command(
    OUTPUT shell_redirect.out
    VERBATIM
    COMMAND ${CMAKE_COMMAND} -E cat shell_redirect.in
    COMMAND ${CMAKE_COMMAND} -E touch shell_redirect.out
  )
else()
  add_custom_command(
    OUTPUT shell_redirect.out
    VERBATIM
    COMMAND ${CMAKE_COMMAND} -E cat - < shell_redirect.in
    COMMAND ${CMAKE_COMMAND} -E echo "> operator" > shell_redirect.out
    COMMAND ${CMAKE_COMMAND} -E echo ">> operator" >> shell_redirect.out
  )
endif()

set_property(SOURCE output1 output2 output3 PROPERTY SYMBOLIC 1)
add_custom_target(customTarget ALL
  COMMAND ${CMAKE_COMMAND} -E true
  DEPENDS output1 output3 shell_redirect.out
)
add_test(NAME test COMMAND $<TARGET_FILE:main>)
if(DISABLE_TEST)
  set_tests_properties(test PROPERTIES DISABLED TRUE)
endif()
install(TARGETS main)
set_target_properties(main PROPERTIES LABELS "label1;label2")
set_target_properties(lib PROPERTIES LABELS "label3")

if(LONG_CUSTOM_COMMAND_OUTPUT)
  file(GLOB_RECURSE LONG_CUSTOM_FILES
    RELATIVE "${CMAKE_ROOT}"
    "${CMAKE_ROOT}/Modules/*")
  set(long_dst "${CMAKE_CURRENT_BINARY_DIR}/Destination-Modules")
  set(long_outs)
  foreach(f IN LISTS LONG_CUSTOM_FILES)
    # Generate outputs without spaces to correct expansion
    list(APPEND long_outs "Destination-${f}")
  endforeach()
  add_custom_command(
    OUTPUT ${long_outs}
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_ROOT}/Modules" "${long_dst}"
    COMMAND "${CMAKE_COMMAND}" -E touch "${long_dst}/check-done.txt"
    COMMENT "Copying Modules to Destination-Modules"
  )
  add_custom_target(longCustomCommand ALL DEPENDS ${long_outs})
endif()

if (FAIL)
  file(WRITE "${CMAKE_BINARY_DIR}/dummy.c"
    "#error \"something which will not compile\"\n"
  )
  add_executable(dummy "${CMAKE_BINARY_DIR}/dummy.c")
  add_test(NAME dummy COMMAND ${CMAKE_COMMAND} -E false)
  install(CODE "message(FATAL_ERROR \"Failed install.\")")
endif()

if (INTERRUPT_BUILD_SRC)
  add_executable(InterruptBuild "${INTERRUPT_BUILD_SRC}")
  add_custom_target(interruptSlow ALL
    COMMAND ${CMAKE_COMMAND} -E echo "interruptSlow: begin"
    COMMAND ${CMAKE_COMMAND} -E sleep 30
    COMMAND ${CMAKE_COMMAND} -E echo "interruptSlow: end"
  )
endif()

if (INSTALL_INTERRUPT)
  # Enable parallel install and add several slow install subdirectories so that
  # a `cmake --install -j 1` runs long enough to be interrupted with install
  # scripts still pending.
  set_property(GLOBAL PROPERTY INSTALL_PARALLEL ON)
  add_subdirectory(installSlow1)
  add_subdirectory(installSlow2)
  add_subdirectory(installSlow3)
endif()

if (CTEST_INTERRUPT)
  # A fast test plus several slow tests.  A high COST on the fast test makes it
  # sort first (CTest orders by descending cost, and on a first run with no
  # historical cost data the COST property is used), so with `ctest -j 1` the
  # fast test finishes first -- recording a completed test in the checkpoint --
  # and then one slow test is in-flight, with the remaining slow tests pending,
  # when the interrupt arrives.  Each test touches a marker so the check script
  # can observe which tests ran.  Dependencies are deliberately avoided: on a
  # `ctest -F` resume the finished fast test is removed, which would otherwise
  # leave its dependents blocked.  (InterruptBuild is provided by the
  # INTERRUPT_BUILD_SRC block above, which is always enabled together with this.)
  set(runtest "${CMAKE_CURRENT_SOURCE_DIR}/runtest.cmake")
  add_test(NAME ctestFast COMMAND ${CMAKE_COMMAND}
    -DMARKER=${CMAKE_BINARY_DIR}/ran/ctestFast -P ${runtest})
  set_tests_properties(ctestFast PROPERTIES COST 100)
  foreach (n 1 2 3)
    add_test(NAME ctestSlow${n} COMMAND ${CMAKE_COMMAND}
      -DMARKER=${CMAKE_BINARY_DIR}/ran/ctestSlow${n} -DSECONDS=10 -P ${runtest})
    set_tests_properties(ctestSlow${n} PROPERTIES COST 1)
  endforeach()
endif()
