enable_language(C)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/main.c" "int main() { return 0;}")

set(include_dir "${CMAKE_CURRENT_BINARY_DIR}/dir")
set(before_include_dir "${CMAKE_CURRENT_BINARY_DIR}/dirBefore")
file(MAKE_DIRECTORY "${include_dir}")
file(MAKE_DIRECTORY "${before_include_dir}")

add_executable(main "${CMAKE_CURRENT_BINARY_DIR}/main.c")
include_directories("${include_dir}")
target_include_directories(main BEFORE PRIVATE "${before_include_dir}")

get_target_property(actual_include_dirs main INCLUDE_DIRECTORIES)
set(desired_include_dirs "${before_include_dir}" "${include_dir}")

if (NOT "${actual_include_dirs}" MATCHES "${desired_include_dirs}")
    message(SEND_ERROR "include before does not work")
endif()
