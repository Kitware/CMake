# Built-in cache documentation structural-parity lint.
#
# The table ``Source/cmCacheDocumentationTable.cxx`` is hand-maintained;
# its set of entries is the curated list of CMake cache variables for
# which ``cmake-gui`` and ``ccmake`` surface a built-in tooltip when the
# on-disk ``HELPSTRING`` is empty (the common case for variables a user
# sets via ``-D`` without an accompanying docstring).
#
# The sibling table ``Source/cmCachePatternTable.cxx`` carries the
# placeholder-shaped fallbacks (``CMAKE_<LANG>_FLAGS``,
# ``CMAKE_POLICY_DEFAULT_CMP<NNNN>``, ``<PROJECT-NAME>_VERSION``, ...).
# Patterns store their placeholder names with angle brackets; the
# corresponding ``Help/variable/<NAME>.rst`` filenames carry the same
# placeholder tokens with the brackets stripped.
#
# Drift between an entry's wording and its source ``Help/variable/*.rst``
# manual is handled by reviewer discipline : when an ``.rst`` is reworded,
# the matching table entry is reviewed in the same change.  This test
# therefore enforces only the one piece of drift that a human reviewer
# would not naturally catch: that every entry NAME in both tables still
# resolves to an existing ``Help/variable/<NAME>.rst`` file (so the
# tooltip is never silently divorced from a real manual via a typo, a
# rename, or a deletion).

if(NOT DEFINED CMake_SOURCE_DIR OR NOT IS_DIRECTORY "${CMake_SOURCE_DIR}")
  message(FATAL_ERROR
    "CacheVarHelpCoverage: CMake_SOURCE_DIR was not provided.")
endif()

set(_help_dir     "${CMake_SOURCE_DIR}/Help/variable")
set(_table_file   "${CMake_SOURCE_DIR}/Source/cmCacheDocumentationTable.cxx")
set(_pattern_file "${CMake_SOURCE_DIR}/Source/cmCachePatternTable.cxx")

if(NOT IS_DIRECTORY "${_help_dir}")
  message(FATAL_ERROR
    "CacheVarHelpCoverage: ${_help_dir} does not exist.")
endif()
if(NOT EXISTS "${_table_file}")
  message(FATAL_ERROR
    "CacheVarHelpCoverage: ${_table_file} does not exist.")
endif()
if(NOT EXISTS "${_pattern_file}")
  message(FATAL_ERROR
    "CacheVarHelpCoverage: ${_pattern_file} does not exist.")
endif()

# Extract entry names from the C++ exact-match table.  Each entry begins
# with the fixed pattern ``\n  { "NAME",``; see the header comment in
# cmCacheDocumentationTable.cxx for the layout convention.
file(READ "${_table_file}" _table_text)
string(REGEX MATCHALL "\n  \\{ \"[^\"]+\"" _hits "${_table_text}")
set(_table_names)
foreach(_hit IN LISTS _hits)
  string(REGEX REPLACE "^\n  \\{ \"" "" _name "${_hit}")
  string(REGEX REPLACE "\"$" "" _name "${_name}")
  list(APPEND _table_names "${_name}")
endforeach()
list(SORT _table_names)

# Extract entry patterns from the C++ pattern table.  Same row shape as
# above; the pattern strings carry angle-bracketed placeholders like
# ``CMAKE_<LANG>_FLAGS`` or ``<PROJECT-NAME>_VERSION``.  Strip the
# angle brackets to derive the matching ``.rst`` basename
# (``CMAKE_LANG_FLAGS.rst`` etc.).
file(READ "${_pattern_file}" _pattern_text)
string(REGEX MATCHALL "\n  \\{ \"[^\"]+\"" _phits "${_pattern_text}")
set(_pattern_names)
foreach(_hit IN LISTS _phits)
  string(REGEX REPLACE "^\n  \\{ \"" "" _name "${_hit}")
  string(REGEX REPLACE "\"$" "" _name "${_name}")
  list(APPEND _pattern_names "${_name}")
endforeach()
list(SORT _pattern_names)

# Table entries whose NAME does not match an existing
# Help/variable/<NAME>.rst manual.
set(_missing_exact)
foreach(_name IN LISTS _table_names)
  if(NOT EXISTS "${_help_dir}/${_name}.rst")
    list(APPEND _missing_exact "${_name}")
  endif()
endforeach()

# Pattern entries whose NAME (with ``<`` and ``>`` stripped) does not
# match an existing Help/variable/<NAME>.rst manual.
set(_missing_pattern)
foreach(_name IN LISTS _pattern_names)
  string(REPLACE "<" "" _basename "${_name}")
  string(REPLACE ">" "" _basename "${_basename}")
  if(NOT EXISTS "${_help_dir}/${_basename}.rst")
    list(APPEND _missing_pattern "${_name}")
  endif()
endforeach()

if(_missing_exact OR _missing_pattern)
  set(_msg "Built-in cache documentation coverage check failed.")
  if(_missing_exact)
    list(LENGTH _missing_exact _n)
    string(APPEND _msg
      "\n\n${_n} entry/entries in Source/cmCacheDocumentationTable.cxx"
      "\nhave no matching Help/variable/<NAME>.rst manual:")
    foreach(_name IN LISTS _missing_exact)
      string(APPEND _msg "\n  ${_name}")
    endforeach()
  endif()
  if(_missing_pattern)
    list(LENGTH _missing_pattern _n)
    string(APPEND _msg
      "\n\n${_n} entry/entries in Source/cmCachePatternTable.cxx"
      "\nhave no matching Help/variable/<NAME>.rst manual (the "
      "placeholder tokens carry through verbatim with ``<``/``>`` "
      "stripped):")
    foreach(_name IN LISTS _missing_pattern)
      string(REPLACE "<" "" _basename "${_name}")
      string(REPLACE ">" "" _basename "${_basename}")
      string(APPEND _msg "\n  ${_name} -> ${_basename}.rst")
    endforeach()
  endif()
  string(APPEND _msg
    "\n\nLikely causes and remedies:"
    "\n  * The entry NAME / Pattern is misspelled -- fix the spelling."
    "\n  * Help/variable/<NAME>.rst was renamed -- update the entry to"
    "\n    match the new filename."
    "\n  * Help/variable/<NAME>.rst was deleted intentionally -- remove"
    "\n    the corresponding entry from the table.")
  message(FATAL_ERROR "${_msg}")
endif()

list(LENGTH _table_names _n_tab)
list(LENGTH _pattern_names _n_pat)
message(STATUS
  "Built-in cache documentation coverage OK: "
  "${_n_tab} exact entry/entries + ${_n_pat} pattern entry/entries, "
  "all matched to Help/variable/*.rst.")
