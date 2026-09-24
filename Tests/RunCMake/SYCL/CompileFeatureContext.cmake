set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
enable_language(CXX SYCL)

set(cxx_constexpr_standard "")
set(sycl_constexpr_standard "")
foreach(level IN ITEMS 11 14 17 20 23 26)
  set(level_features "${CMAKE_CXX${level}_COMPILE_FEATURES}")
  list(FIND level_features cxx_constexpr feature_index)
  if(feature_index GREATER_EQUAL 0 AND NOT cxx_constexpr_standard)
    set(cxx_constexpr_standard "${level}")
  endif()

  set(level_features "${CMAKE_SYCL${level}_COMPILE_FEATURES}")
  list(FIND level_features cxx_constexpr feature_index)
  if(feature_index GREATER_EQUAL 0 AND NOT sycl_constexpr_standard)
    set(sycl_constexpr_standard "${level}")
  endif()
endforeach()
if(NOT cxx_constexpr_standard OR NOT sycl_constexpr_standard)
  message(FATAL_ERROR "cxx_constexpr is not available for both CXX and SYCL")
endif()

add_library(cxx_feature STATIC feature.cxx)
set_property(TARGET cxx_feature PROPERTY CXX_STANDARD 98)
target_compile_features(cxx_feature PRIVATE cxx_constexpr)
get_property(cxx_standard TARGET cxx_feature PROPERTY CXX_STANDARD)
get_property(cxx_sycl_standard TARGET cxx_feature PROPERTY SYCL_CXX_STANDARD)
if(NOT cxx_standard STREQUAL cxx_constexpr_standard)
  message(FATAL_ERROR
    "CXX target selected CXX_STANDARD '${cxx_standard}', expected "
    "'${cxx_constexpr_standard}'")
endif()
if(cxx_sycl_standard)
  message(FATAL_ERROR
    "CXX target unexpectedly selected SYCL_CXX_STANDARD '${cxx_sycl_standard}'")
endif()

add_library(sycl_feature STATIC feature.sycl)
set_property(TARGET sycl_feature PROPERTY SYCL_CXX_STANDARD 98)
target_compile_features(sycl_feature PRIVATE cxx_constexpr)
get_property(sycl_standard TARGET sycl_feature PROPERTY SYCL_CXX_STANDARD)
get_property(sycl_cxx_standard TARGET sycl_feature PROPERTY CXX_STANDARD)
if(NOT sycl_standard STREQUAL sycl_constexpr_standard)
  message(FATAL_ERROR
    "SYCL target selected SYCL_CXX_STANDARD '${sycl_standard}', expected "
    "'${sycl_constexpr_standard}'")
endif()
if(sycl_cxx_standard)
  message(FATAL_ERROR
    "SYCL target unexpectedly selected CXX_STANDARD '${sycl_cxx_standard}'")
endif()

add_library(mixed_file_set_feature STATIC)
target_sources(mixed_file_set_feature PRIVATE
  FILE_SET cxx_sources TYPE SOURCES FILES feature.cxx
  FILE_SET sycl_sources TYPE SYCL FILES feature.sycl)
set_property(TARGET mixed_file_set_feature PROPERTY CXX_STANDARD 98)
set_property(TARGET mixed_file_set_feature PROPERTY SYCL_CXX_STANDARD 98)
target_compile_features(mixed_file_set_feature PRIVATE cxx_constexpr)
get_property(mixed_cxx_standard TARGET mixed_file_set_feature
  PROPERTY CXX_STANDARD)
get_property(mixed_sycl_standard TARGET mixed_file_set_feature
  PROPERTY SYCL_CXX_STANDARD)
if(NOT mixed_cxx_standard STREQUAL cxx_constexpr_standard)
  message(FATAL_ERROR
    "Mixed file-set target selected CXX_STANDARD '${mixed_cxx_standard}', "
    "expected '${cxx_constexpr_standard}'")
endif()
if(NOT mixed_sycl_standard STREQUAL "98")
  message(FATAL_ERROR
    "Mixed file-set target changed SYCL_CXX_STANDARD to '${mixed_sycl_standard}'")
endif()

add_library(sycl_headers_feature STATIC feature.cxx)
target_sources(sycl_headers_feature PRIVATE
  FILE_SET SYCL_HEADERS FILES sycl_header.h)
set_property(TARGET sycl_headers_feature PROPERTY CXX_STANDARD 98)
set_property(TARGET sycl_headers_feature PROPERTY SYCL_CXX_STANDARD 98)
target_compile_features(sycl_headers_feature PRIVATE cxx_constexpr)
get_property(headers_cxx_standard TARGET sycl_headers_feature
  PROPERTY CXX_STANDARD)
get_property(headers_sycl_standard TARGET sycl_headers_feature
  PROPERTY SYCL_CXX_STANDARD)
if(NOT headers_cxx_standard STREQUAL "98")
  message(FATAL_ERROR
    "SYCL headers target changed CXX_STANDARD to '${headers_cxx_standard}'")
endif()
if(NOT headers_sycl_standard STREQUAL sycl_constexpr_standard)
  message(FATAL_ERROR
    "SYCL headers target selected SYCL_CXX_STANDARD '${headers_sycl_standard}', "
    "expected '${sycl_constexpr_standard}'")
endif()
