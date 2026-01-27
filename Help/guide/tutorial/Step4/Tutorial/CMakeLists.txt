add_executable(Tutorial)

target_sources(Tutorial
  PRIVATE
    Tutorial.cxx
)

# TODO14: Add VendorLib to Tutorial

target_link_libraries(Tutorial
  PRIVATE
    MathFunctions
)

# TODO4: Add a compile feature for C++20 support to Tutorial


if(
  (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR
  (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
)

  # TODO9: Add the /W3 compile flag to Tutorial

elseif(
  (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
  (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
)

  # TODO10: Add the -Wall compile flag to Tutorial


endif()
