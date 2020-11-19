cmake_policy(SET CMP0119 NEW)
include(CMP0119-Common.cmake)

enable_language(CXX)
add_executable(AltExtCXX AltExtCXX.zzz)
set_property(SOURCE AltExtCXX.zzz PROPERTY LANGUAGE CXX)
