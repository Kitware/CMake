enable_language(CXX)

include(WarningAsErrorOptions.cmake)
get_warning_options(warning_options)

if (DEFINED warning_options)
  add_executable(WerrorOn warn.cxx)
  target_compile_options(WerrorOn PUBLIC "${warning_options}")
  set_target_properties(WerrorOn PROPERTIES COMPILE_WARNING_AS_ERROR ON)
else()
  # if no werror option is set for the environment, use err.cxx so that build fails as expected
  add_executable(WerrorOn err.cxx)
endif()
