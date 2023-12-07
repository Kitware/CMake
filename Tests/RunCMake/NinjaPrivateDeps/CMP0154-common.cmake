enable_language(CXX)

function(copy_file file dest)
  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/${dest}
    COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_SOURCE_DIR}/${file} ${CMAKE_BINARY_DIR}/${dest}
  )
endfunction()

copy_file(header.h.in private.h)
copy_file(header.h.in public.h)
copy_file(source.cpp.in empty.cpp)
copy_file(source.cpp.in none.cpp)

add_library(HelloLib_PrivateFileSet STATIC hello_lib.cpp)
target_sources(HelloLib_PrivateFileSet
  PRIVATE FILE_SET HEADERS
    BASE_DIRS ${CMAKE_BINARY_DIR}
    FILES ${CMAKE_BINARY_DIR}/private.h
)

add_library(HelloLib_PublicFileSet STATIC hello_lib.cpp)
target_sources(HelloLib_PublicFileSet
  PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_BINARY_DIR}
    FILES ${CMAKE_BINARY_DIR}/public.h
)

add_library(HelloLib_EmptyFileSet STATIC hello_lib.cpp empty.cpp)
target_sources(HelloLib_EmptyFileSet
  PUBLIC FILE_SET HEADERS
)

add_library(HelloLib_NoFileSet STATIC hello_lib.cpp none.cpp)

function(hello_executable name)
  add_executable(Hello_${name} hello.cpp)
  target_link_libraries(Hello_${name} PRIVATE HelloLib_${name})
endfunction()

hello_executable(PrivateFileSet)
hello_executable(PublicFileSet)
hello_executable(EmptyFileSet)
hello_executable(NoFileSet)
