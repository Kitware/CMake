
cmake_minimum_required (VERSION 2.8.12)
project(Producer)

add_library(fakezlib SHARED fakezlib.cpp)
set_property(TARGET fakezlib PROPERTY SOVERSION 1)
set_property(TARGET fakezlib PROPERTY VERSION 1.2.7)
set_property(TARGET fakezlib PROPERTY LIBRARY_OUTPUT_DIRECTORY ${sysroot}/lib/${triple})
set_property(TARGET fakezlib PROPERTY OUTPUT_NAME z)

file(MAKE_DIRECTORY "${sysroot}/usr/lib/${triple}/")
add_custom_command(TARGET fakezlib POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E remove "${sysroot}/lib/${triple}/libz.so"
  COMMAND ${CMAKE_COMMAND} -E create_symlink "${sysroot}/lib/${triple}/libz.so.1.2.7" "${sysroot}/usr/lib/${triple}/libz.so"
)
file(COPY "fakezlib.h" DESTINATION "${CMAKE_BINARY_DIR}")
file(RENAME "${CMAKE_BINARY_DIR}/fakezlib.h" "${CMAKE_BINARY_DIR}/zlib.h")

install(FILES "${CMAKE_BINARY_DIR}/zlib.h" DESTINATION "${sysroot}/usr/include/")
