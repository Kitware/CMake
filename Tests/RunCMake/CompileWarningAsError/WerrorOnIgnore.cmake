enable_language(${LANGUAGE})

include(WarningAsErrorOptions.cmake)
get_warning_options(warning_options ${LANGUAGE})

if(NOT DEFINED FILENAME)
  set(FILENAME warn)
endif()

add_executable(WerrorOn ${FILENAME}.${EXTENSION})
target_compile_options(WerrorOn PUBLIC "${warning_options}")
set_target_properties(WerrorOn PROPERTIES COMPILE_WARNING_AS_ERROR ON)
