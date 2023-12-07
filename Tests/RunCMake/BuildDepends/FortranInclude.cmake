enable_language(Fortran)

if("${CMAKE_Fortran_COMPILER_ID};${CMAKE_Fortran_SIMULATE_ID}" MATCHES "^Intel(LLVM)?;MSVC$")
  string(APPEND CMAKE_Fortran_FLAGS_DEBUG " -Z7")
  string(APPEND CMAKE_Fortran_FLAGS_RELWITHDEBINFO " -Z7")
endif()

set(check_pairs "")

add_executable(preprocess FortranIncludePreprocess.F)
set_property(TARGET preprocess PROPERTY Fortran_PREPROCESS ON)
target_include_directories(preprocess PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND check_pairs "$<TARGET_FILE:preprocess>|${CMAKE_CURRENT_BINARY_DIR}/preprocess.inc")

# LCC < 1.24 has no way to disable Fortran preprocessor
if(NOT CMAKE_Fortran_COMPILER_ID STREQUAL "LCC" OR CMAKE_Fortran_COMPILER_VERSION VERSION_GREATER_EQUAL "1.24.00")
  add_executable(no_preprocess FortranIncludeNoPreprocess.f)
  set_property(TARGET no_preprocess PROPERTY Fortran_PREPROCESS OFF)
  target_include_directories(no_preprocess PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  list(APPEND check_pairs "$<TARGET_FILE:no_preprocess>|${CMAKE_CURRENT_BINARY_DIR}/no_preprocess.inc")
endif()

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake CONTENT "
set(check_pairs \"${check_pairs}\")
")
