enable_language(C)
enable_testing()

add_test(NAME DoesNotUseLauncher
  COMMAND ${CMAKE_COMMAND} -E echo "Hi")

add_executable(exe main.c)
get_property(test_launcher TARGET exe PROPERTY TEST_LAUNCHER)
set_property(TARGET exe PROPERTY TEST_LAUNCHER "$<1:${test_launcher}>")

add_test(NAME UsesTestLauncher
  COMMAND exe)

add_test(NAME DoesNotUseTestLauncherWithGenex
  COMMAND $<TARGET_FILE:exe>)

add_subdirectory(TestLauncher)

add_test(NAME UsesTestLauncherWithExecTargetFromSubdirAddedWithoutGenex
  COMMAND subdir_exe_no_genex)

add_test(NAME DoesNotUseTestLauncherWithExecTargetFromSubdirAddedWithGenex
  COMMAND $<TARGET_FILE:subdir_exe_with_genex>)

add_executable(local_launcher ../pseudo_emulator.c)
add_executable(use_launcher_local main.c)
set_property(TARGET use_launcher_local PROPERTY TEST_LAUNCHER "$<TARGET_FILE:local_launcher>")
add_test(NAME UsesLocalLauncher COMMAND use_launcher_local)
