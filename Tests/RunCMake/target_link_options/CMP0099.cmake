
enable_language(C)

set(obj "${CMAKE_C_OUTPUT_EXTENSION}")
if(BORLAND)
  set(pre -)
endif()

add_library(LinkOptions_interface INTERFACE)
target_link_options (LinkOptions_interface INTERFACE ${pre}BADFLAG_INTERFACE${obj})

add_library(LinkOptions_static1 STATIC LinkOptionsLib.c)
target_link_libraries (LinkOptions_static1 PRIVATE LinkOptions_interface)

add_library(LinkOptions_static2 STATIC LinkOptionsLib.c)
target_link_libraries (LinkOptions_static2 PRIVATE LinkOptions_static1)

add_executable(LinkOptions_exe LinkOptionsExe.c)
target_link_libraries (LinkOptions_exe PRIVATE LinkOptions_static2)
