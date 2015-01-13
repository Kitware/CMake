project(JsonCpp CXX)

set(JSONCPP_SOURCES
  src/lib_json/json_batchallocator.h
  src/lib_json/json_reader.cpp
  src/lib_json/json_tool.h
  src/lib_json/json_value.cpp
  src/lib_json/json_valueiterator.inl
  src/lib_json/json_writer.cpp
  )

include_directories(
  ${JsonCpp_SOURCE_DIR}/include
  ${KWSYS_HEADER_ROOT}
  )

add_library(cmjsoncpp ${JSONCPP_SOURCES})
