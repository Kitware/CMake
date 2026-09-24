cmake_minimum_required(VERSION 4.4)
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
project(CompileFlags SYCL)
add_executable(SyclOnlyCompileFlags main.sycl)
# SYCL has no native architecture property yet; test the compiler-option part
# of CudaOnly.CompileFlags without substituting an architecture abstraction.
target_compile_options(SyclOnlyCompileFlags PRIVATE -DALWAYS_DEFINE)
