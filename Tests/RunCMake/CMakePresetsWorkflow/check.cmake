set(CMakePresets_VALIDATE_SCRIPT_PATH "${RunCMake_SOURCE_DIR}/../CMakePresets/validate_schema.py")
include("${RunCMake_SOURCE_DIR}/../CMakePresets/validate_schema.cmake")
include("${RunCMake_SOURCE_DIR}/../CMakePresets/check.cmake")
