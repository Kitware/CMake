add_executable(Tutorial)

target_sources(Tutorial
  PRIVATE
    Tutorial.cxx
)

target_link_libraries(Tutorial
  PRIVATE
    MathFunctions
)

target_compile_features(Tutorial PRIVATE cxx_std_20)

if(
  (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR
  (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
)

  target_compile_options(Tutorial PRIVATE /W3)

elseif(
  (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
  (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
)

  target_compile_options(Tutorial PRIVATE -Wall)

endif()
