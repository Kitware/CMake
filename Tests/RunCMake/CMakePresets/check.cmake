if(NOT (Python_EXECUTABLE AND CMake_TEST_JSON_SCHEMA))
  return()
endif()

include("${RunCMake_SOURCE_DIR}/../validate_json_schema.cmake")

if(NOT CMakePresets_SCHEMA_EXPECTED_RESULT)
  set(CMakePresets_SCHEMA_EXPECTED_RESULT 0)
endif()

set(CMakePresets_JSON_SCHEMA "${RunCMake_SOURCE_DIR}/../../../Help/manual/presets/schema.json")

if(EXISTS "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json")
  validate_json_schema(
    "${CMakePresets_JSON_SCHEMA}"
    "${RunCMake_TEST_SOURCE_DIR}/CMakePresets.json"
    EXPECTED_RESULT "${CMakePresets_SCHEMA_EXPECTED_RESULT}"
  )
endif()

if(NOT CMakeUserPresets_SCHEMA_EXPECTED_RESULT)
  set(CMakeUserPresets_SCHEMA_EXPECTED_RESULT 0)
endif()
if(EXISTS "${RunCMake_TEST_SOURCE_DIR}/CMakeUserPresets.json")
  validate_json_schema(
    "${CMakePresets_JSON_SCHEMA}"
    "${RunCMake_TEST_SOURCE_DIR}/CMakeUserPresets.json"
    EXPECTED_RESULT "${CMakeUserPresets_SCHEMA_EXPECTED_RESULT}"
  )
endif()

if(NOT CMakePresets_EXTRA_FILES_SCHEMA_EXPECTED_RESULTS)
  set(CMakePresets_EXTRA_FILES_SCHEMA_EXPECTED_RESULTS "${_CMakePresets_EXTRA_FILES_SCHEMA_EXPECTED_RESULTS}")
endif()
foreach(_f _r IN ZIP_LISTS _CMakePresets_EXTRA_FILES_OUT CMakePresets_EXTRA_FILES_SCHEMA_EXPECTED_RESULTS)
  validate_json_schema(
    "${CMakePresets_JSON_SCHEMA}" "${_f}"
    EXPECTED_RESULT "${_r}"
  )
endforeach()
