install(CODE "set(ENV{DESTDIR} \"${CMAKE_CURRENT_BINARY_DIR}/root-all\")")
install(FILES empty.c DESTINATION ".")
