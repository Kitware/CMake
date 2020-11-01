cmake_minimum_required(VERSION 3.14)
project(VerifyFindOpenAcc Fortran)

find_package(OpenACC REQUIRED)

add_executable(UsesOpenACC main.f90)
target_link_libraries(UsesOpenACC PRIVATE OpenACC::OpenACC_Fortran)

add_executable(UsesOpenACCVars main.f90)
target_link_options(UsesOpenACCVars PRIVATE ${OpenACC_Fortran_OPTIONS})
target_compile_options(UsesOpenACCVars PRIVATE ${OpenACC_Fortran_OPTIONS})
