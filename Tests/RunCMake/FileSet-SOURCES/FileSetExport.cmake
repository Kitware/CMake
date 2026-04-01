enable_language(C)

add_library(lib1 STATIC)
target_sources(lib1 PRIVATE lib1.c)
target_sources(lib1 PUBLIC FILE_SET a TYPE SOURCES FILES lib2.c)

set_property(FILE_SET a TARGET lib1 PROPERTY COMPILE_DEFINITIONS LIB1_A)
set_property(FILE_SET a TARGET lib1 PROPERTY COMPILE_OPTIONS -DOPT_LIB1_A)
set_property(FILE_SET a TARGET lib1 PROPERTY INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/subdir1>"
                                                                  "$<INSTALL_INTERFACE:include/subdir1>")

set_property(FILE_SET a TARGET lib1 PROPERTY INTERFACE_COMPILE_DEFINITIONS INTERFACE_LIB1_A)
set_property(FILE_SET a TARGET lib1 PROPERTY INTERFACE_COMPILE_OPTIONS -DOPT_INTERFACE_LIB1_A)
set_property(FILE_SET a TARGET lib1 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/subdir2>"
                                                                           "$<INSTALL_INTERFACE:include/subdir2>")

target_sources(lib1 PUBLIC FILE_SET SOURCES FILES lib5.c)

set_property(FILE_SET SOURCES TARGET lib1 PROPERTY INTERFACE_COMPILE_DEFINITIONS INTERFACE_LIB1_SRCS)
set_property(FILE_SET SOURCES TARGET lib1 PROPERTY INTERFACE_COMPILE_OPTIONS -DOPT_INTERFACE_LIB1_SRCS)
set_property(FILE_SET SOURCES TARGET lib1 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/srcs>"
                                                                                 "$<INSTALL_INTERFACE:include/srcs>")


add_executable(main main.c)
target_link_libraries(main PRIVATE lib1)
target_compile_definitions(main PRIVATE CONSUMER)

if(CMAKE_GENERATOR MATCHES "Xcode")
  install(TARGETS lib1 EXPORT export FILE_SET a DESTINATION sources
                                     FILE_SET SOURCES DESTINATION sources/srcs)
else()
  install(TARGETS lib1 EXPORT export FILE_SET a DESTINATION sources
                                     FILE_SET SOURCES DESTINATION sources/$<IF:$<CONFIG:Debug>,debug,release>)
endif()
install(FILES subdir1/h1.h subdir1/h3.h DESTINATION include/subdir1)
install(FILES subdir2/h2.h subdir2/h3.h DESTINATION include/subdir2)

install(EXPORT export FILE export.cmake NAMESPACE install:: DESTINATION lib/cmake)
export(EXPORT export FILE export.cmake NAMESPACE export::)
