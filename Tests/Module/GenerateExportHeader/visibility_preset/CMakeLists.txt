
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

if (CMAKE_CXX_FLAGS MATCHES "-fvisibility=hidden")
  message(SEND_ERROR "Do not use add_compiler_export_flags before adding this directory")
endif()

add_library(visibility_preset SHARED visibility_preset.cpp)
generate_export_header(visibility_preset)

add_executable(visibility_preset_exe main.cpp)

target_link_libraries(visibility_preset_exe visibility_preset)
