enable_language(C)

set(out "${CMAKE_CURRENT_BINARY_DIR}/folder")
set(link_folder "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CMakeTmp")
set(link_dir "${link_folder}/link_dir")
file(MAKE_DIRECTORY "${out}")
file(MAKE_DIRECTORY "${link_folder}")
file(WRITE ${out}/empty_file "")
file(CREATE_LINK ${out} ${link_dir} SYMBOLIC)

try_compile(res ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src.c)

if(EXISTS ${link_dir})
  message(FATAL_ERROR "did not remove ${link_dir}")
endif()
if(NOT EXISTS ${out})
  message(FATAL_ERROR "should not have removed ${out}/dir")
endif()

file(REMOVE_RECURSE "${out}")
