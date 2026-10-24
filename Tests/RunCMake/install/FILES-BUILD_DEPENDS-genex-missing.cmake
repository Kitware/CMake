install(FILES empty.c DESTINATION .
  BUILD_DEPENDS "$<$<BOOL:OFF>:missing_off>" "$<1:missing_target>")
