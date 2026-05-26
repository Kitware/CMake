# Inspects ${RunCMake_TEST_BINARY_DIR}/CMakeCache.txt and verifies that
# ``cmake::ProcessCacheArg`` installed the expected HELPSTRING (from the
# built-in documentation table) and preserved the cache entry's type
# and value verbatim.
#
# Each entry's HELPSTRING is serialized as zero or more ``//``-prefixed
# lines immediately preceding the ``KEY:TYPE=VALUE`` line.

set(_cache_file "${RunCMake_TEST_BINARY_DIR}/CMakeCache.txt")
if(NOT EXISTS "${_cache_file}")
  set(RunCMake_TEST_FAILED "Missing cache file: ${_cache_file}")
  return()
endif()

file(READ "${_cache_file}" _cache_text)
string(REGEX REPLACE "\r\n" "\n" _cache_text "${_cache_text}")
string(REPLACE ";" "\\;" _cache_text "${_cache_text}")
string(REPLACE "\n" ";" _cache_lines "${_cache_text}")

# Collect the contiguous run of ``//`` comment lines that immediately
# precedes ``${entry_key}:`` and join their bodies (everything after the
# leading ``//``) into a single string, one body per line.  An entry
# preceded by no ``//`` lines yields the empty string.
function(_collect_help entry_key out_var)
  set(_collected "")
  set(_run "")
  foreach(_line IN LISTS _cache_lines)
    if(_line MATCHES "^//(.*)$")
      string(APPEND _run "${CMAKE_MATCH_1}\n")
    elseif(_line MATCHES "^${entry_key}:[A-Za-z]+=")
      set(_collected "${_run}")
      break()
    else()
      set(_run "")
    endif()
  endforeach()
  set(${out_var} "${_collected}" PARENT_SCOPE)
endfunction()

# Find the ``${entry_key}:TYPE=VALUE`` line itself (the full type+value
# record, without the leading ``//`` help block).  Sets ``out_var`` to
# the empty string if the entry is absent.
function(_find_entry_line entry_key out_var)
  set(_found "")
  foreach(_line IN LISTS _cache_lines)
    if(_line MATCHES "^${entry_key}:[A-Za-z]+=")
      set(_found "${_line}")
      break()
    endif()
  endforeach()
  set(${out_var} "${_found}" PARENT_SCOPE)
endfunction()

if(test STREQUAL "DefineSetsTypeAndHelp")
  # Bare ``-D CMAKE_COMPILE_WARNING_AS_ERROR=ON`` -- ProcessCacheArg
  # must install the documentation-table summary as HELPSTRING and
  # leave the entry typed UNINITIALIZED (the user did not pin a type).
  _find_entry_line("CMAKE_COMPILE_WARNING_AS_ERROR" _line)
  _collect_help("CMAKE_COMPILE_WARNING_AS_ERROR" _help)
  if(NOT _line MATCHES "^CMAKE_COMPILE_WARNING_AS_ERROR:UNINITIALIZED=ON$")
    set(RunCMake_TEST_FAILED
      "ProcessCacheArg unexpectedly altered the bare -D entry's type "
      "or value.\nRecovered entry line:\n[${_line}]")
  elseif(NOT _help MATCHES "treat warnings on compile as errors")
    set(RunCMake_TEST_FAILED
      "ProcessCacheArg did not install HELPSTRING from documentation "
      "table.\nRecovered help block:\n[${_help}]")
  endif()
elseif(test STREQUAL "DefineTypePreserved")
  # ``-D CMAKE_COMPILE_WARNING_AS_ERROR:STRING=ON`` -- the user's
  # explicit ``:STRING`` annotation is honored; HELPSTRING is still
  # populated from the table because the user supplied none.
  _find_entry_line("CMAKE_COMPILE_WARNING_AS_ERROR" _line)
  _collect_help("CMAKE_COMPILE_WARNING_AS_ERROR" _help)
  if(NOT _line MATCHES "^CMAKE_COMPILE_WARNING_AS_ERROR:STRING=ON$")
    set(RunCMake_TEST_FAILED
      "ProcessCacheArg overwrote user's explicit :STRING type pin.\n"
      "Recovered entry line:\n[${_line}]")
  elseif(NOT _help MATCHES "treat warnings on compile as errors")
    set(RunCMake_TEST_FAILED
      "ProcessCacheArg did not install HELPSTRING when user pinned a "
      "type.\nRecovered help block:\n[${_help}]")
  endif()
elseif(test STREQUAL "DefineUnknownVarUninitialized")
  # ``-D MY_CUSTOM_VAR_XYZZY=x`` -- a project-specific name absent from
  # the documentation table.  Type stays UNINITIALIZED and HELPSTRING
  # falls back to the legacy sentinel.
  _find_entry_line("MY_CUSTOM_VAR_XYZZY" _line)
  _collect_help("MY_CUSTOM_VAR_XYZZY" _help)
  if(NOT _line MATCHES "^MY_CUSTOM_VAR_XYZZY:UNINITIALIZED=x$")
    set(RunCMake_TEST_FAILED
      "ProcessCacheArg altered the unknown-variable entry.\n"
      "Recovered entry line:\n[${_line}]")
  elseif(NOT _help MATCHES "No help, variable specified on the command line")
    set(RunCMake_TEST_FAILED
      "ProcessCacheArg did not preserve sentinel HELPSTRING for unknown "
      "variable.\nRecovered help block:\n[${_help}]")
  endif()
elseif(test STREQUAL "PatternCxxClangTidy")
  # ``-D CMAKE_CXX_CLANG_TIDY=clang-tidy`` -- no exact-match entry in
  # ``cmCacheDocumentationTable``; the pattern table's
  # ``CMAKE_<LANG>_CLANG_TIDY`` entry matches.  ProcessCacheArg installs
  # the pattern's Summary as HELPSTRING; type stays UNINITIALIZED.
  _find_entry_line("CMAKE_CXX_CLANG_TIDY" _line)
  _collect_help("CMAKE_CXX_CLANG_TIDY" _help)
  if(NOT _line MATCHES "^CMAKE_CXX_CLANG_TIDY:UNINITIALIZED=clang-tidy$")
    set(RunCMake_TEST_FAILED
      "ProcessCacheArg unexpectedly altered the pattern-matched entry's "
      "type or value.\nRecovered entry line:\n[${_line}]")
  elseif(NOT _help MATCHES "clang-tidy command")
    set(RunCMake_TEST_FAILED
      "Pattern-table lookup did not install HELPSTRING from the "
      "CMAKE_<LANG>_CLANG_TIDY pattern.\n"
      "Recovered help block:\n[${_help}]")
  endif()
elseif(test STREQUAL "PatternHipCppcheck")
  # ``-D CMAKE_HIP_CPPCHECK=anything`` -- the matcher is intentionally
  # permissive about ``<LANG>`` even though the cppcheck pattern's
  # Summary names a C/CXX restriction.  Assert the value is preserved
  # verbatim, the type stays UNINITIALIZED, and the Summary's
  # restriction phrase appears in the HELPSTRING.
  _find_entry_line("CMAKE_HIP_CPPCHECK" _line)
  _collect_help("CMAKE_HIP_CPPCHECK" _help)
  if(NOT _line MATCHES "^CMAKE_HIP_CPPCHECK:UNINITIALIZED=anything$")
    set(RunCMake_TEST_FAILED
      "ProcessCacheArg unexpectedly altered the pattern-matched entry's "
      "type or value.\nRecovered entry line:\n[${_line}]")
  elseif(NOT _help MATCHES "C or CXX")
    set(RunCMake_TEST_FAILED
      "Pattern-table lookup did not surface the C/CXX restriction in "
      "the HELPSTRING for CMAKE_HIP_CPPCHECK.\n"
      "Recovered help block:\n[${_help}]")
  endif()
else()
  set(RunCMake_TEST_FAILED "Unknown test case: ${test}")
endif()
