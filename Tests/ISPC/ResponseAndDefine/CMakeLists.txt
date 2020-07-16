
project(ispc_spaces_in_path)

set(CMAKE_NINJA_FORCE_RESPONSE_FILE ON)

# Make sure we can handle an arg file with tricky defines including spaces in -I include
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/path with spaces/simple_include.h"
"
  typedef float FLOAT_TYPE;
"
)

add_executable(SpacesInPath main.cxx simple.ispc)
set_target_properties(SpacesInPath PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(SpacesInPath PRIVATE  "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options(SpacesInPath PRIVATE
  "$<$<COMPILE_LANGUAGE:ISPC>:--target=sse2-i32x4;--arch=x86-64>")

target_compile_definitions(SpacesInPath PRIVATE
  "$<$<COMPILE_LANGUAGE:ISPC>:STRUCT_DEFINE=\"struct{uniform int a;varying int b;\";M_PI=3.1415926535f>")
target_include_directories(SpacesInPath PRIVATE
  "$<$<COMPILE_LANGUAGE:ISPC>:\"fake path with spaces\">""
  "$<$<COMPILE_LANGUAGE:ISPC>:\"path with spaces\">")
