enable_language(C)
enable_testing()

add_test(NAME DoesNotUseLauncher
  COMMAND ${CMAKE_COMMAND} -E echo "Hi")

add_executable(exe main.c)

add_test(NAME UsesTestLauncher
  COMMAND exe)

add_test(NAME DoesNotUseTestLauncherWithGenex
  COMMAND $<TARGET_FILE:exe>)

add_subdirectory(TestLauncher)

add_test(NAME UsesTestLauncherWithExecTargetFromSubdirAddedWithoutGenex
  COMMAND subdir_exe_no_genex)

add_test(NAME DoesNotUseTestLauncherWithExecTargetFromSubdirAddedWithGenex
  COMMAND $<TARGET_FILE:subdir_exe_with_genex>)
