enable_language(${LANGUAGE})

include(WarningAsErrorOptions.cmake)
get_warning_options(warning_options ${LANGUAGE})

add_executable(WerrorOn warn.${EXTENSION})
target_compile_options(WerrorOn PUBLIC "${warning_options}")
set_target_properties(WerrorOn PROPERTIES COMPILE_WARNING_AS_ERROR ON)
