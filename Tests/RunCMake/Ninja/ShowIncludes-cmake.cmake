# Set the console code page.
execute_process(COMMAND cmd /c chcp "${CODEPAGE}")

if(VSLANG)
  set(ENV{VSLANG} "${VSLANG}")
endif()

if(RunCMake_MAKE_PROGRAM)
  set(maybe_MAKE_PROGRAM "-DCMAKE_MAKE_PROGRAM=${RunCMake_MAKE_PROGRAM}")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" . -G Ninja ${maybe_MAKE_PROGRAM})
