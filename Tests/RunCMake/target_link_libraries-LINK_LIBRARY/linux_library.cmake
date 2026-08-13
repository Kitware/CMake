
enable_language(C)

add_library(lib SHARED base.c lib.c)

# feature NEEDED_LIBRARY
add_executable(main-needed_library main.c)
target_link_directories(main-needed_library PRIVATE "${RunCMake_BINARY_DIR}/linux_library_external-build"
                                                     "${RunCMake_BINARY_DIR}/linux_library_external-build/$<CONFIG>")
target_link_libraries(main-needed_library PRIVATE "$<LINK_LIBRARY:NEEDED_LIBRARY,lib,external>")
