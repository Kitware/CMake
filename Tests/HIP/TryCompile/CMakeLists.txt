cmake_minimum_required(VERSION 3.18)
project (TryCompile HIP)

#Goal for this example:
# Verify try_compile with HIP language works
set(CMAKE_HIP_STANDARD 14)

if(CMAKE_HIP_PLATFORM STREQUAL "amd")
  set(CMAKE_HIP_ARCHITECTURES gfx803 gfx900)
elseif(CMAKE_HIP_PLATFORM STREQUAL "nvidia")
  set(CMAKE_HIP_ARCHITECTURES 52)
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
try_compile(result "${CMAKE_CURRENT_BINARY_DIR}"
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/device_function.hip"
        COPY_FILE "${CMAKE_CURRENT_BINARY_DIR}/device_function.o")

add_executable(HIPOnlyTryCompile main.hip)
target_link_libraries(HIPOnlyTryCompile "${CMAKE_CURRENT_BINARY_DIR}/device_function.o")
