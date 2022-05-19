enable_language(CXX)

include(WarningAsErrorOptions.cmake)
get_warning_options(warning_options)

add_executable(WerrorOn warn.cxx)
target_compile_options(WerrorOn PUBLIC "${warning_options}")
set_target_properties(WerrorOn PROPERTIES COMPILE_WARNING_AS_ERROR ON)
