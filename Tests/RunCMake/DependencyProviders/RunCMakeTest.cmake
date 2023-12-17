include(RunCMake)

run_cmake_with_options(BeforeProject
  -D "include_before_project=set_provider.cmake"
  -D "provider_command=null_provider"
  -D "provider_methods=find_package"
)
run_cmake_with_options(AfterProject
  -D "include_after_project=set_provider.cmake"
  -D "provider_command=null_provider"
  -D "provider_methods=find_package"
)
run_cmake_with_options(ProjectIncludeBefore
  -D "CMAKE_PROJECT_INCLUDE_BEFORE=set_provider.cmake"
  -D "provider_command=null_provider"
  -D "provider_methods=find_package"
)
run_cmake_with_options(ProjectIncludeAfter
  -D "CMAKE_PROJECT_INCLUDE=set_provider.cmake"
  -D "provider_command=null_provider"
  -D "provider_methods=find_package"
)
run_cmake_with_options(ToolchainFile
  -D "CMAKE_TOOLCHAIN_FILE=set_provider.cmake"
  -D "provider_command=null_provider"
  -D "provider_methods=find_package"
)
run_cmake_with_options(NoCommand
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=set_provider.cmake"
  -D "provider_methods=find_package"
)
run_cmake_with_options(NoMethods
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=set_provider.cmake"
  -D "provider_command=null_provider"
)
run_cmake_with_options(NoCommandOrMethods
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=set_provider.cmake"
)
run_cmake_with_options(PassThroughProvider
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=set_provider.cmake"
  -D "provider_command=null_provider"
  -D "provider_methods=FIND_PACKAGE\\;FETCHCONTENT_MAKEAVAILABLE_SERIAL"
)
run_cmake_with_options(FindPackage
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=set_provider.cmake"
  -D "provider_command=find_package_provider"
  -D "provider_methods=FIND_PACKAGE"
)
run_cmake_with_options(RedirectFindPackage
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=set_provider.cmake"
  -D "provider_command=redirect_find_package_provider"
  -D "provider_methods=FIND_PACKAGE"
)
run_cmake_with_options(FetchContentSerial
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=set_provider.cmake"
  -D "provider_command=FetchContentSerial_provider"
  -D "provider_methods=FETCHCONTENT_MAKEAVAILABLE_SERIAL"
)
run_cmake_with_options(RedirectFetchContentSerial
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=set_provider.cmake"
  -D "provider_command=redirect_FetchContentSerial_provider"
  -D "provider_methods=FETCHCONTENT_MAKEAVAILABLE_SERIAL"
)
run_cmake_with_options(ProviderFirst
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=set_provider.cmake"
  -D "provider_command=FetchContentSerial_provider"
  -D "provider_methods=FETCHCONTENT_MAKEAVAILABLE_SERIAL"
)
run_cmake_with_options(Bypass
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=set_provider.cmake"
  -D "provider_command=forward_find_package"
  -D "provider_methods=FIND_PACKAGE"
)
run_cmake_with_options(Recurse
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=set_provider.cmake"
  -D "provider_command=recurse_FetchContent"
  -D "provider_methods=FETCHCONTENT_MAKEAVAILABLE_SERIAL"
)
