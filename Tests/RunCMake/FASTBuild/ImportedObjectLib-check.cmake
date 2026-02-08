# Check that we're linking to an object file on disk
# and not to "-objects" alias (which only exists for non-imported targets).
set(REGEX_TO_MATCH "
.*.Libraries =
.*{
.*'MyApp_CXX_Objs_1',
.*dummy.o'
.*}
")
include(${RunCMake_SOURCE_DIR}/check.cmake)
