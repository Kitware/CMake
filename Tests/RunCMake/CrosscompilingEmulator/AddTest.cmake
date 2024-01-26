enable_language(C)
enable_testing()
set(CMAKE_CROSSCOMPILING 1)

add_test(NAME DoesNotUseEmulator
  COMMAND ${CMAKE_COMMAND} -E echo "Hi")

add_executable(exe main.c)

add_test(NAME UsesEmulator
  COMMAND exe)

add_test(NAME DoesNotUseEmulatorWithGenex
  COMMAND $<TARGET_FILE:exe>)

add_subdirectory(AddTest)

add_test(NAME UsesEmulatorWithExecTargetFromSubdirAddedWithoutGenex
  COMMAND subdir_exe_no_genex)

add_test(NAME DoesNotUseEmulatorWithExecTargetFromSubdirAddedWithGenex
  COMMAND $<TARGET_FILE:subdir_exe_with_genex>)

add_executable(exe_test_launcher main.c)
set_property(TARGET exe_test_launcher PROPERTY TEST_LAUNCHER "pseudo_test_launcher")

add_test(NAME UsesTestLauncherAndEmulator
  COMMAND exe_test_launcher)
