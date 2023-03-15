cmake_policy(SET CMP0067 NEW)
enable_language(CXX)

# Add our own -std= flag to the try_compile check.
set(CMAKE_REQUIRED_FLAGS -std=c++11)

# Tell CMP0128 NEW behavior to append a -std= flag (after ours).
if(CMAKE_CXX_EXTENSIONS_DEFAULT)
  set(CMAKE_CXX_EXTENSIONS OFF)
else()
  set(CMAKE_CXX_EXTENSIONS ON)
endif()

include(CheckSourceCompiles)
check_source_compiles(CXX "
${check_cxx_std}
int main()
{
  return 0;
}
" SRC_COMPILED)
if(NOT SRC_COMPILED)
  message("Check failed to compile:")
  set(configure_log "${CMAKE_BINARY_DIR}/CMakeFiles/CMakeConfigureLog.yaml")
  if(EXISTS "${configure_log}")
    file(READ "${configure_log}" log_content)
  else()
    set(log_content "")
  endif()
  if(log_content MATCHES [[(  -
    kind: "try_compile-v1"(
+    [^
]+)+
    checks:
      - "Performing Test SRC_COMPILED"(
+    [^
]+)+)]])
    message("${configure_log} contains:\n${CMAKE_MATCH_1}")
  endif()
endif()
