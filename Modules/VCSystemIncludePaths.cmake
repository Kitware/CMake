file(WRITE
     ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/vc_system_include_paths.cxx
     "#ifndef __cplusplus\n"
     "# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"\n"
     "#endif\n"
     "int main () { }\n")

try_compile(VC_SYSTEM_INCLUDE_PATHS_RETURN
            ${CMAKE_BINARY_DIR}
            ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/vc_system_include_paths.cxx
            OUTPUT_VARIABLE
            VC_SYSTEM_INCLUDE_PATHS_OUTPUT)

unset(VC_SYSTEM_INCLUDE_PATHS_RETURN)
