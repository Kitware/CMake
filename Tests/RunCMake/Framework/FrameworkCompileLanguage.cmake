enable_language(CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Framework search paths must follow the include directories seen by the
# language being compiled, not those seen by C.
set(fw_c "${CMAKE_CURRENT_BINARY_DIR}/fw_c")
set(fw_cxx "${CMAKE_CURRENT_BINARY_DIR}/fw_cxx")
file(MAKE_DIRECTORY "${fw_c}/C.framework" "${fw_cxx}/CXX.framework")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/main.cxx" "int main() { return 0; }\n")

# Reach the frameworks transitively so they are not also added as include
# directories of main through its direct link items.
add_library(frameworks INTERFACE)
target_link_libraries(frameworks INTERFACE "${fw_c}/C.framework" "${fw_cxx}/CXX.framework")

add_executable(main "${CMAKE_CURRENT_BINARY_DIR}/main.cxx")
target_include_directories(main PRIVATE
  "$<$<COMPILE_LANGUAGE:C>:${fw_c}/C.framework>"
  "$<$<COMPILE_LANGUAGE:CXX>:${fw_cxx}/CXX.framework>"
)
target_link_libraries(main PRIVATE frameworks)
