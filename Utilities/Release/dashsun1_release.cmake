set(PROCESSORS 1)
set(HOST dashsun1)
set(MAKE "make")
execute_process(COMMAND ssh ${HOST}
  pwd RESULT_VARIABLE result OUTPUT_VARIABLE BUILD_DIR)
# now strip the newline (we need perl chop...)
string(LENGTH "${BUILD_DIR}" length)
math(EXPR length "${length} -1" )
string(SUBSTRING "${BUILD_DIR}" 0 ${length} BUILD_DIR)
set(USER_MAKE_RULE_FILE
  "${BUILD_DIR}/CMakeReleaseDirectory/UserMakeRules.cmake")
set(INITIAL_CACHE "
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
CMAKE_BUILD_TYPE:STRING=Release
HAVE_LIBDL:INTERNAL=FALSE
CMAKE_EXE_LINKER_FLAGS:STRING=-Bdynamic -ldl -Bstatic
CMAKE_USER_MAKE_RULES_OVERRIDE:STRING=${USER_MAKE_RULE_FILE}
CURSES_LIBRARY:FILEPATH=/usr/lib/libcurses.a
FORM_LIBRARY:FILEPATH=/usr/lib/libform.a")
set(USER_MAKE_RULE_FILE_CONTENTS
  "SET(CMAKE_DL_LIBS \\\\\"-Bdynamic -ldl -Bstatic\\\\\")")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
