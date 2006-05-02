set(PROCESSORS 2)
set(HOST dashsun1)
set(MAKE "make -j2")
execute_process(COMMAND ssh ${HOST} pwd RESULT_VARIABLE result OUTPUT_VARIABLE BUILD_DIR)
message(fatal_error ${BUILD_DIR}/CMakeReleaseDirectory/UserMakeRules.cmake)
set(INITIAL_CACHE "
CMAKE_EXE_LINKER_FLAGS:STRING=-Bstatic
CMAKE_USER_MAKE_RULES_OVERRIDE:STRING=${BINDIR}/UserMakeRules.cmake
CURSES_LIBRARY:FILEPATH=/usr/lib/libcurses.a
FORM_LIBRARY:FILEPATH=/usr/lib/libform.a")
set(USER_MAKE_RULES "SET(CMAKE_DL_LIBS \"-Bdynamic -ldl -Bstatic\")"
include(release_cmake.cmake)
