cmake_minimum_required(VERSION 4.4)
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
project(WithDefs SYCL)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(release_compile_defs DEFREL)
add_definitions("-DPACKED_DEFINE=__attribute__((packed))")
add_executable(SyclOnlyWithDefs main.notsycl)
set_source_files_properties(main.notsycl PROPERTIES LANGUAGE SYCL)
target_compile_options(SyclOnlyWithDefs PRIVATE
  -DFLAG_COMPILE_LANG_$<COMPILE_LANGUAGE>
  -DFLAG_LANG_IS_SYCL=$<COMPILE_LANGUAGE:SYCL>)
target_compile_definitions(SyclOnlyWithDefs PRIVATE
  $<$<CONFIG:RELEASE>:$<BUILD_INTERFACE:${release_compile_defs}>>
  DEF_COMPILE_LANG_$<COMPILE_LANGUAGE>
  DEF_LANG_IS_SYCL=$<COMPILE_LANGUAGE:SYCL>
  DEF_SYCL_COMPILER=$<SYCL_COMPILER_ID>
  DEF_SYCL_COMPILER_VERSION=$<SYCL_COMPILER_VERSION>)
target_include_directories(SyclOnlyWithDefs PRIVATE
  $<$<COMPILE_LANGUAGE:SYCL>:${CMAKE_CURRENT_SOURCE_DIR}/inc_sycl>)
