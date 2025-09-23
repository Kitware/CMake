# Read the JSON file into a variable
set(autogenInfoFilePath "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/foo_autogen.dir/AutogenInfo.json")

if(NOT IS_READABLE "${autogenInfoFilePath}")
  set(RunCMake_TEST_FAILED "Expected autogen info file missing:\n \"${autogenInfoFilePath}\"")
  return()
endif()
file(READ "${autogenInfoFilePath}" jsonRaw)

# If multi-config generator, we are looking for MOC_INCLUDES_<CONFIG>.
if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(mocKey "MOC_INCLUDES_Debug") # Pick one arbitrarily (they will all be the same in this test)
# If single-config generator, we are looking for MOC_INCLUDES.
else()
  set(mocKey "MOC_INCLUDES")
endif()

string(JSON actualValue GET "${jsonRaw}" "${mocKey}")

# The format of the MOC_INCLUDES entries in AutogenInfo.json depends on how long the paths are.
# For short entries:
#	"MOC_INCLUDES" : [ "<SHORT_PATH>" ]
# For long entries:
# 	"MOC_INCLUDES_Debug" :
#	[
#		"<SOME_PARTICULARLY_LONG_PATH>"
#	],

# Also, paths given to AUTOMOC_INCLUDE_DIRECTORIES must be absolute paths.
# The code uses SystemTools::FileIsFullPath() to verify this, and it accepts
# a forward slash at the beginning for both Windows (network path) and UNIX platforms.
# Therefore, for the simplicity of this test, use a dummy value "/pass".

# Strip the JSON format around the string for a true before/after comparison.
string(REPLACE "[ \"" "" actualValue ${actualValue})
string(REPLACE "\" ]" "" actualValue ${actualValue})

# Final pass/fail comparison.
set(expectedValue "/pass")

if (NOT actualValue STREQUAL expectedValue)
  set(RunCMake_TEST_FAILED "AUTOMOC_INCLUDE_DIRECTORIES override property not honored.")
  string(APPEND RunCMake_TEST_FAILURE_MESSAGE
    "Expected MOC_INCLUDES in AutogenInfo.json to have ${expectedValue} but found ${actualValue}."
  )
endif()
