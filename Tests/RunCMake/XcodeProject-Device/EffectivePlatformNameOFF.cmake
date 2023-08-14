enable_language(CXX)

set_property(GLOBAL PROPERTY XCODE_EMIT_EFFECTIVE_PLATFORM_NAME OFF)

set(CMAKE_MACOSX_BUNDLE true)

add_library(library STATIC foo.cpp)

add_executable(main main.cpp)
target_link_libraries(main library)

install(TARGETS library ARCHIVE DESTINATION lib)
