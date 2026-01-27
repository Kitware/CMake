set(CMake_TEST_GUI "ON" CACHE BOOL "")
set(CMake_TEST_MODULE_COMPILATION "named,compile_commands,collation,partitions,internal_partitions,export_bmi,install_bmi,shared,bmionly,build_database" CACHE STRING "")
set(CMake_TEST_TLS_VERIFY_URL "https://gitlab.kitware.com" CACHE STRING "")
set(CMake_TEST_TLS_VERIFY_URL_BAD "https://badtls-expired.kitware.com" CACHE STRING "")
set(CMake_TEST_TLS_VERSION "1.3" CACHE STRING "")
set(CMake_TEST_TLS_VERSION_URL_BAD "https://badtls-v1-1.kitware.com:8011" CACHE STRING "")

string(CONCAT coverage_flags
  "--coverage "
  "-fprofile-abs-path "
  # These files are committed generated sources and contain relative path
  # `#line` directives. CI coverage then cannot find the source files reliably.
  # See related issue #20001.
  "-fprofile-exclude-files=cmExprParser[.].*\\;cmFortranParser[.].* "
  "-fdiagnostics-show-option "
  "-Wall "
  "-Wextra "
  "-Wshadow "
  "-Wpointer-arith "
  "-Winvalid-pch "
  "-Wcast-align "
  "-Wdisabled-optimization "
  "-Wwrite-strings "
  "-fstack-protector-all "
  "-Wconversion "
  "-Wno-error=sign-conversion "
  "-Wno-error=conversion ")
string(CONCAT link_flags
  "--coverage ")

set(CMAKE_C_FLAGS "${coverage_flags}" CACHE STRING "")
set(CMAKE_CXX_FLAGS "${coverage_flags} -Woverloaded-virtual -Wstrict-null-sentinel" CACHE STRING "")
# Apply `LDFLAGS`.
set(CMAKE_EXE_LINKER_FLAGS_INIT "${link_flags}" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${link_flags}" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${link_flags}" CACHE STRING "")

# Do not bootstrap for the coverage test suite.
set(CMAKE_SKIP_BOOTSTRAP_TEST TRUE CACHE BOOL "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora43_common.cmake")
