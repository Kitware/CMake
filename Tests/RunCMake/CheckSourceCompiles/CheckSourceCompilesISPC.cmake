
enable_language (ISPC)
include(CheckSourceCompiles)

check_source_compiles(ISPC "I don't build" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid ISPC source didn't fail.")
endif()

check_source_compiles(ISPC [=[

float func(uniform int32, float a)
{
  return a / 2.25;
}
]=]
 SHOULD_BUILD)
if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid ISPC source.")
endif()
