enable_language(C)
enable_testing()
set(CMAKE_CROSSCOMPILING 1)

add_test(NAME DoesNotUseEmulator
  COMMAND ${CMAKE_COMMAND} -E echo "Hi")

add_executable(exe main.c)
get_property(emulator TARGET exe PROPERTY CROSSCOMPILING_EMULATOR)
set_property(TARGET exe PROPERTY CROSSCOMPILING_EMULATOR "$<1:${emulator}>")

add_test(NAME UsesEmulator
  COMMAND exe)

add_test(NAME DoesNotUseEmulatorWithGenex
  COMMAND $<TARGET_FILE:exe>)

add_subdirectory(AddTest)

add_test(NAME UsesEmulatorWithExecTargetFromSubdirAddedWithoutGenex
  COMMAND subdir_exe_no_genex)

add_test(NAME DoesNotUseEmulatorWithExecTargetFromSubdirAddedWithGenex
  COMMAND $<TARGET_FILE:subdir_exe_with_genex>)

set(CMAKE_TEST_LAUNCHER "$<1:${CMAKE_CROSSCOMPILING_EMULATOR}>")
add_executable(exe_test_launcher main.c)
unset(CMAKE_TEST_LAUNCHER)

add_test(NAME UsesTestLauncherAndEmulator
  COMMAND exe_test_launcher)

add_executable(local_emulator ../pseudo_emulator.c)
add_executable(use_emulator_local main.c)
set_property(TARGET use_emulator_local PROPERTY CROSSCOMPILING_EMULATOR "$<TARGET_FILE:local_emulator>")
add_test(NAME UsesLocalEmulator COMMAND use_emulator_local)
