
include ("${RunCMake_TEST_BINARY_DIR}/LINKER_TYPE_OPTION.cmake")

# In some environment, `=` character is escaped
string(REPLACE "=" "\\\\?=" LINKER_TYPE_OPTION "${LINKER_TYPE_OPTION}")

if (NOT actual_stdout MATCHES "${LINKER_TYPE_OPTION}")
    set (RunCMake_TEST_FAILED "Not found expected '${LINKER_TYPE_OPTION}'.")
endif()
