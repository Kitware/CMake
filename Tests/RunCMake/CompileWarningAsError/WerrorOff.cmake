enable_language(${LANGUAGE})

include(WarningAsErrorOptions.cmake)
get_warning_options(warning_options ${LANGUAGE})

add_executable(WerrorOff warn.${EXTENSION})
target_compile_options(WerrorOff PUBLIC "${warning_options}")
set_target_properties(WerrorOff PROPERTIES COMPILE_WARNING_AS_ERROR OFF)
