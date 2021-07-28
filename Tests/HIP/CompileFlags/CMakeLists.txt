cmake_minimum_required(VERSION 3.18)
project(CompileFlags HIP)

add_executable(HIPOnlyCompileFlags main.hip)

set_property(TARGET HIPOnlyCompileFlags PROPERTY HIP_ARCHITECTURES gfx803)

target_compile_options(HIPOnlyCompileFlags PRIVATE -DALWAYS_DEFINE)
