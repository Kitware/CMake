
enable_language(C)

add_library(lib SHARED base.c lib.c)

# feature NEEDED_FRAMEWORK
add_executable(main-needed_library main.c)
target_link_directories(main-needed_library PRIVATE "${RunCMake_BINARY_DIR}/apple_library_external-build"
                                                     "${RunCMake_BINARY_DIR}/apple_library_external-build/$<CONFIG>")
target_link_libraries(main-needed_library PRIVATE "$<LINK_LIBRARY:NEEDED_LIBRARY,lib,external>")


# feature REEXPORT_FRAMEWORK
add_executable(main-reexport_library main.c)
target_link_directories(main-reexport_library PRIVATE "${RunCMake_BINARY_DIR}/apple_library_external-build"
                                                      "${RunCMake_BINARY_DIR}/apple_library_external-build/$<CONFIG>")
target_link_libraries(main-reexport_library PRIVATE "$<LINK_LIBRARY:REEXPORT_LIBRARY,lib,external>")


# feature WEAK_FRAMEWORK
add_executable(main-weak_library main.c)
target_link_directories(main-weak_library PRIVATE "${RunCMake_BINARY_DIR}/apple_library_external-build"
                                                  "${RunCMake_BINARY_DIR}/apple_library_external-build/$<CONFIG>")
target_link_libraries(main-weak_library PRIVATE "$<LINK_LIBRARY:WEAK_LIBRARY,lib,external>")
