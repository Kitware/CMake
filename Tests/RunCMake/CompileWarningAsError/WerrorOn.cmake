enable_language(${LANGUAGE})

include(WarningAsErrorOptions.cmake)
get_warning_options(warning_options ${LANGUAGE})

if(NOT DEFINED FILENAME)
  set(FILENAME warn)
endif()

if (DEFINED warning_options)
  add_executable(WerrorOn ${FILENAME}.${EXTENSION})
  target_compile_options(WerrorOn PUBLIC "${warning_options}")
  set_target_properties(WerrorOn PROPERTIES COMPILE_WARNING_AS_ERROR ON)
else()
  # if no werror option is set for the environment, use err so that build fails as expected
  add_executable(WerrorOn err.${EXTENSION})
endif()
