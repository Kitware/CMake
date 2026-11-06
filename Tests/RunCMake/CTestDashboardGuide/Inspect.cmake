enable_language(C)
enable_language(CXX)

set(can_asan FALSE)
if(CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
  include(CheckSourceCompiles)
  set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
  check_source_compiles(C "int main(void) { return 0; }" HAVE_ASAN)
  if(HAVE_ASAN)
    set(can_asan TRUE)
  endif()
endif()

set(can_valgrind FALSE)
find_program(VALGRIND NAMES valgrind NO_CACHE)
if(VALGRIND)
  set(can_valgrind TRUE)
endif()

set(can_sh FALSE)
find_program(SH NAMES sh NO_CACHE)
if(SH)
  set(can_sh TRUE)
endif()

# Find a gcov whose version matches the compiler. The first gcov in the PATH
# may belong to a different GCC, and cannot read the compiler's coverage data.
set(can_code_coverage FALSE)
set(GCOV "")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  file(REAL_PATH "${CMAKE_CXX_COMPILER}" compiler_real)
  set(gcov_names)
  set(hints)

  # Assemble a list of possible gcov names and hints based on the compiler
  # directory.
  foreach(compiler IN ITEMS "${CMAKE_CXX_COMPILER}" "${compiler_real}")
    cmake_path(GET compiler PARENT_PATH dir)
    list(APPEND hints "${dir}")
    # Map a name like x86_64-linux-gnu-g++-12 to x86_64-linux-gnu-gcov-12.
    cmake_path(GET compiler FILENAME compiler_filename)
    if(compiler_filename MATCHES "^(.*)g\\+\\+(.*)$")
      list(APPEND gcov_names "${CMAKE_MATCH_1}gcov${CMAKE_MATCH_2}")
    endif()
  endforeach()

  # Append the version-specific name and clean up the list.
  string(REGEX MATCH "^[0-9]+" major "${CMAKE_CXX_COMPILER_VERSION}")
  list(APPEND gcov_names "gcov-${major}" "gcov")
  list(REMOVE_DUPLICATES gcov_names)

  # Find the first gcov from the above candidates that matches the compiler
  # version.
  foreach(name IN LISTS gcov_names)
    unset(gcov)
    find_program(gcov NAMES "${name}" HINTS ${hints} NO_CACHE)
    if(gcov)
      execute_process(COMMAND "${gcov}" --version
        OUTPUT_VARIABLE version
        RESULT_VARIABLE result
        ERROR_QUIET
      )
      string(FIND "${version}" " ${CMAKE_CXX_COMPILER_VERSION}" pos)
      if(result EQUAL 0 AND NOT pos EQUAL -1)
        set(GCOV "${gcov}")
        set(can_code_coverage TRUE)
        break()
      endif()
    endif()
  endforeach()
endif()

set(info "")
foreach(var
  can_asan
  can_valgrind
  VALGRIND
  can_sh
  SH
  can_code_coverage
  GCOV
)
  string(APPEND info "set(${var} \"${${var}}\")\n")
endforeach()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/info.cmake" "${info}")
