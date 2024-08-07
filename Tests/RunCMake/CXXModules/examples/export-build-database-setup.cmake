set(CMAKE_EXPERIMENTAL_EXPORT_BUILD_DATABASE "4bd552e2-b7fb-429a-ab23-c83ef53f3f13")

get_property(is_multiconfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (is_multiconfig)
  set(CMAKE_CONFIGURATION_TYPES "Debug" "Release")
endif ()

set(CMAKE_EXPORT_BUILD_DATABASE 1)

# Mock up depfile flags to keep things consistent; we don't need accurate
# dependency tracking for this test case anyways.
set(CMAKE_CXX_DEPFILE_FORMAT gcc)
set(CMAKE_DEPFILE_FLAGS_CXX "-Ddepflag=\\\"<DEP_FILE>\\\"")
unset(CMAKE_CXX_DEPFILE_EXTENSION_REPLACE)

# Disable MSVC flag injection from CMake abstractions.
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "")

if (CMAKE_CXX_MODULE_BMI_ONLY_FLAG MATCHES "ifcOutput")
  # Make a single flag for BMI-only to make the JSON expectations simpler.
  set(CMAKE_CXX_MODULE_BMI_ONLY_FLAG
    "-ifcOnly;-ifcOutput<OBJECT>")
endif ()

# Disable extensions to keep flag selection simpler.
set(CMAKE_CXX_EXTENSIONS 0)

if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  set(output_flag "-Fo")
else ()
  set(output_flag "-o")
endif ()
set(CMAKE_CXX_COMPILE_OBJECT
  "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> ${output_flag}<OBJECT> -c <SOURCE>")

set(CMAKE_CXX_FLAGS "-Dfrom_cmake_cxx_flags")
set(CMAKE_CXX_FLAGS_DEBUG "-Dfrom_cmake_cxx_debug_flags")
set(CMAKE_CXX_FLAGS_RELEASE "-Dfrom_cmake_cxx_release_flags")
