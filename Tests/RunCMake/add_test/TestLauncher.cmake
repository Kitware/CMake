project(test_launcher LANGUAGES C)

enable_testing()
add_test(NAME DoesNotUseLauncher
  COMMAND ${CMAKE_COMMAND} -E echo "Hi")

add_executable(generated_exe simple_src_exiterror.cxx)
set_target_properties(generated_exe PROPERTIES LINKER_LANGUAGE C)

add_test(NAME UsesTestLauncher
  COMMAND generated_exe)

add_test(NAME DoesNotUseTestLauncherWithGenex
  COMMAND $<TARGET_FILE:generated_exe>)

add_subdirectory(TestLauncher)

add_test(NAME UsesTestLauncherWithExecTargetFromSubdirAddedWithoutGenex
  COMMAND generated_exe_in_subdir_added_to_test_without_genex)

add_test(NAME DoesNotUseTestLauncherWithExecTargetFromSubdirAddedWithGenex
  COMMAND $<TARGET_FILE:generated_exe_in_subdir_added_to_test_with_genex>)
