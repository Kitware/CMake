set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GenerateExportHeader)

# Show that the export header defined with C++11 attributes exports and hides
# symbols declared with the macros in leading position.

add_library(libattributes SHARED libattributes.cpp)

generate_export_header(libattributes CXX_ATTRIBUTES)

# Show that consumers in language modes without C++11 attributes fall back to
# the compiler-specific spelling.  Generators such as FASTBuild compile the
# output of the Clang preprocessor, whose line markers are a GNU extension.
set(no_line_marker_errors
  $<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>:-Wno-gnu-line-marker>
  $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-Wno-gnu-line-marker>
)

if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
  add_executable(libattributes_c main.c)
  set_property(TARGET libattributes_c PROPERTY C_STANDARD 99)
  target_compile_options(libattributes_c PRIVATE
    -pedantic-errors ${no_line_marker_errors})
  target_link_libraries(libattributes_c PRIVATE libattributes)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
  add_executable(libattributes_cxx98 main.cpp)
  set_target_properties(libattributes_cxx98 PROPERTIES
    CXX_STANDARD 98
    CXX_EXTENSIONS OFF
  )
  target_compile_options(libattributes_cxx98 PRIVATE
    -pedantic-errors ${no_line_marker_errors})
  target_link_libraries(libattributes_cxx98 PRIVATE libattributes)
endif()
