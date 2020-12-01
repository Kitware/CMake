enable_language(C)
try_compile(_result ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR}/main.c)
find_package(ThisPackageHopefullyDoesNotExist CONFIG)
