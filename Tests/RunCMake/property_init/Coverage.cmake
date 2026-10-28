# Check that every property initialized from `StaticTargetProperties` in
# `Source/cmTarget.cxx` is tested by the case file matching its
# `InitCondition`, and that every property tested here is still initialized.

set(source_file "${CMake_SOURCE_DIR}/Source/cmTarget.cxx")
if (NOT EXISTS "${source_file}")
  message(FATAL_ERROR
    "Source file not found: '${source_file}'")
endif ()

# Map each `InitCondition` to the case file testing it.
set(case_for_Always                         "Always")
set(case_for_CanCompileSources              "CompileSources")
set(case_for_NeedsXcodeAndCanCompileSources "CompileSources")
set(case_for_NormalTarget                   "NormalTarget")
set(case_for_NonImportedTarget              "NonImportedTarget")
set(case_for_NormalNonImportedTarget        "NonImportedNormalTarget")
set(case_for_TargetWithArtifact             "TargetsWithArtifact")
set(case_for_NonExecutableWithArtifact      "LibraryArtifact")
set(case_for_LinkableLibraryTarget          "Linkable")
set(case_for_ExecutableTarget               "Executable")
set(case_for_SharedLibraryTarget            "SharedLibrary")
set(case_for_TargetWithSymbolExports        "TargetsWithExports")
set(case_for_TargetWithCommands             "TargetsWithCommands")

# Properties tested by a case other than the one for their `InitCondition`.
set(case_override_POSITION_INDEPENDENT_CODE "PICTargets")

# Properties in the table which are not tested.
set(untested
  # property                  reason
  "XCODE_GENERATE_SCHEME"     "NeedsXcode: no Xcode-only, all-target-type case"
  )

# Properties tested which are not initialized from the table.
set(not_in_table
  # property                  reason
  "notset"                    "Never initialized, checks the harness itself"
  "SYSTEM"                    "Initialized separately for non-normal targets"
  )

# Extract the property names from a "property reason" table.
function (prepare_properties_table table output)
  set(_properties)
  set(is_property 1)
  foreach (item IN LISTS "${table}")
    if (is_property)
      list(APPEND _properties "${item}")
      set(is_property 0)
    else ()
      set(is_property 1)
    endif ()
  endforeach ()
  set("${output}" "${_properties}" PARENT_SCOPE)
endfunction ()

# Parse the source table.
file(READ "${source_file}" source)
string(REPLACE ";" "" source "${source}")
string(REPLACE "\\" "" source "${source}")

string(FIND "${source}" "#define COMMON_LANGUAGE_PROPERTIES(lang)" macro_begin)
string(FIND "${source}" "TargetProperty const StaticTargetProperties[] = {" table_begin)
if (macro_begin EQUAL -1 OR table_begin EQUAL -1)
  message(FATAL_ERROR
    "Failed to find `StaticTargetProperties` in '${source_file}'")
endif ()
math(EXPR macro_length "${table_begin} - ${macro_begin}")
string(SUBSTRING "${source}" "${macro_begin}" "${macro_length}" macro_text)
string(SUBSTRING "${source}" "${table_begin}" -1 table_text)
string(FIND "${table_text}" "\n};" table_end)
string(SUBSTRING "${table_text}" 0 "${table_end}" table_text)

set(language_suffixes)
string(REPLACE "\n" ";" macro_lines "${macro_text}")
foreach (line IN LISTS macro_lines)
  if (line MATCHES "\\{ #lang \"([A-Za-z_]+)\"_s, IC::([A-Za-z]+)")
    list(APPEND language_suffixes "${CMAKE_MATCH_1}")
    set("language_ic_${CMAKE_MATCH_1}" "${CMAKE_MATCH_2}")
  endif ()
endforeach ()

set(source_properties)
string(REPLACE "\n" ";" table_lines "${table_text}")
foreach (line IN LISTS table_lines)
  if (line MATCHES "^ *\\{ \"([A-Za-z_]+)\"_s(, \"[^\"]*\"_s)?(, IC::([A-Za-z]+))?")
    set(property "${CMAKE_MATCH_1}")
    set(ic "${CMAKE_MATCH_4}")
    if (ic STREQUAL "")
      set(ic "Always")
    endif ()
    list(APPEND source_properties "${property}")
    set("source_ic_${property}" "${ic}")
  elseif (line MATCHES "^ *COMMON_LANGUAGE_PROPERTIES\\(([A-Za-z]+)\\)")
    set(lang "${CMAKE_MATCH_1}")
    foreach (suffix IN LISTS language_suffixes)
      list(APPEND source_properties "${lang}${suffix}")
      set("source_ic_${lang}${suffix}" "${language_ic_${suffix}}")
    endforeach ()
  endif ()
endforeach ()

if (NOT source_properties)
  message(FATAL_ERROR
    "Failed to parse any properties from '${source_file}'")
endif ()

# Parse the test tables. Rows look like `"PROPERTY" "expected" "alias"`.
file(GLOB case_files "${CMAKE_CURRENT_LIST_DIR}/*.cmake")
list(REMOVE_ITEM case_files
  "${CMAKE_CURRENT_LIST_DIR}/Coverage.cmake"
  "${CMAKE_CURRENT_LIST_DIR}/RunCMakeTest.cmake"
  "${CMAKE_CURRENT_LIST_DIR}/util.cmake"
)

set(tested_properties)
foreach (case_file IN LISTS case_files)
  get_filename_component(case "${case_file}" NAME_WE)
  file(STRINGS "${case_file}" lines)
  set(language_templates)
  set(languages)
  foreach (line IN LISTS lines)
    if (line MATCHES "^ *\"([A-Za-z_]+)\" +\"")
      list(APPEND tested_properties "${CMAKE_MATCH_1}")
      list(APPEND "tested_cases_${CMAKE_MATCH_1}" "${case}")
    elseif (line MATCHES "^ *\"\\\${lang}([A-Za-z_]+)\" +\"")
      list(APPEND language_templates "${CMAKE_MATCH_1}")
    elseif (line MATCHES "^add_language_properties\\(([A-Za-z]+) ")
      list(APPEND languages "${CMAKE_MATCH_1}")
    endif ()
  endforeach ()
  foreach (lang IN LISTS languages)
    foreach (suffix IN LISTS language_templates)
      list(APPEND tested_properties "${lang}${suffix}")
      list(APPEND "tested_cases_${lang}${suffix}" "${case}")
    endforeach ()
  endforeach ()
endforeach ()
list(REMOVE_DUPLICATES tested_properties)

# Every source property must be tested in its case (or be listed as untested).
prepare_properties_table(untested untested_properties)
foreach (property IN LISTS source_properties)
  set(ic "${source_ic_${property}}")
  if (DEFINED "case_override_${property}")
    set(case "${case_override_${property}}")
  elseif (DEFINED "case_for_${ic}")
    set(case "${case_for_${ic}}")
  else ()
    set(case "")
  endif ()

  if (property IN_LIST untested_properties)
    if (DEFINED "tested_cases_${property}")
      message(SEND_ERROR
        "Property '${property}' is listed as untested in Coverage.cmake, but "
        "is tested in: ${tested_cases_${property}}")
    endif ()
  elseif (case STREQUAL "")
    message(SEND_ERROR
      "Property '${property}' uses `IC::${ic}`, which has no case mapped in "
      "Coverage.cmake")
  elseif (NOT DEFINED "tested_cases_${property}")
    message(SEND_ERROR
      "Property '${property}' (`IC::${ic}`) is initialized in cmTarget.cxx, "
      "but is not tested in ${case}.cmake")
  elseif (NOT case IN_LIST "tested_cases_${property}")
    message(SEND_ERROR
      "Property '${property}' (`IC::${ic}`) should be tested in "
      "${case}.cmake, but is tested in: ${tested_cases_${property}}")
  endif ()
endforeach ()

# Every tested property must still be initialized from the table.
prepare_properties_table(not_in_table not_in_table_properties)
foreach (property IN LISTS tested_properties)
  if (NOT property IN_LIST source_properties AND
      NOT property IN_LIST not_in_table_properties)
    message(SEND_ERROR
      "Property '${property}' is tested in ${tested_cases_${property}}, but "
      "is not initialized from `StaticTargetProperties` in cmTarget.cxx")
  endif ()
endforeach ()
