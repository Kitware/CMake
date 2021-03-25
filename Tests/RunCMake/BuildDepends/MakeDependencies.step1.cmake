file(TOUCH "${RunCMake_TEST_BINARY_DIR}/main.c")
foreach(i RANGE 1 20000)
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/temp_header_file_${i}.h"
    "#define HEADER_${i} ${i}\n"
    )
  file(APPEND "${RunCMake_TEST_BINARY_DIR}/main.c"
    "#include \"temp_header_file_${i}.h\"\n"
    )
endforeach()
file(APPEND "${RunCMake_TEST_BINARY_DIR}/main.c"
  "#include \"main.h\"\n"
  )
file(APPEND "${RunCMake_TEST_BINARY_DIR}/main.c"
  "int main(void) { return COUNT; }\n"
  )
file(WRITE "${RunCMake_TEST_BINARY_DIR}/main.h"
  "#define COUNT 1\n"
  )
