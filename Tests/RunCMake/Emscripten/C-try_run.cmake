enable_language(C)
try_run(RUN_RESULT
  COMPILE_RESULT
  SOURCE_FROM_CONTENT main.c "int main(void) { return 12; }\n"
  NO_CACHE
)
message(STATUS "COMPILE_RESULT='${COMPILE_RESULT}'")
message(STATUS "RUN_RESULT='${RUN_RESULT}'")
