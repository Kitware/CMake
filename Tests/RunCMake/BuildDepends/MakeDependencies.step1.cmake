file(TOUCH "${RunCMake_TEST_BINARY_DIR}/main.c")
if(RunCMake_GENERATOR STREQUAL "Borland Makefiles")
  set(num_headers 2000)
else()
  set(num_headers 20000)
endif()
foreach(i RANGE 1 ${num_headers})
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
