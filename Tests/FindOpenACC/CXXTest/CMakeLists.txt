cmake_minimum_required(VERSION 3.14)
project(VerifyFindOpenAcc CXX)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenACC REQUIRED)

add_executable(UsesOpenACC main.cxx)
target_link_libraries(UsesOpenACC PRIVATE OpenACC::OpenACC_CXX)

add_executable(UsesOpenACCVars main.cxx)
target_link_options(UsesOpenACCVars PRIVATE ${OpenACC_CXX_OPTIONS})
target_compile_options(UsesOpenACCVars PRIVATE ${OpenACC_CXX_OPTIONS})
