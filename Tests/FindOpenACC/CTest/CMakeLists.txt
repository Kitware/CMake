cmake_minimum_required(VERSION 3.14)
project(VerifyFindOpenAcc C)

set(CMAKE_C_STANDARD 11)

find_package(OpenACC REQUIRED)

add_executable(UsesOpenACC main.c)
target_link_libraries(UsesOpenACC PRIVATE OpenACC::OpenACC_C)

add_executable(UsesOpenACCVars main.c)
target_link_options(UsesOpenACCVars PRIVATE ${OpenACC_C_OPTIONS})
target_compile_options(UsesOpenACCVars PRIVATE ${OpenACC_C_OPTIONS})
