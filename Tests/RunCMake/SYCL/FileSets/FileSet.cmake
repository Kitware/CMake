set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
enable_language(C CXX)
enable_language(SYCL)

set_source_files_properties(FileSetSource.cxx PROPERTIES LANGUAGE CXX)

add_library(fileset_cxx OBJECT FileSetSource.cxx)
target_compile_definitions(fileset_cxx PRIVATE
  EXPECT_CXX
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

add_library(fileset_sycl OBJECT)
target_sources(fileset_sycl PRIVATE
  FILE_SET sycl_sources TYPE SYCL FILES FileSetSource.cxx)
target_compile_definitions(fileset_sycl PRIVATE
  EXPECT_SYCL
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

get_property(type FILE_SET sycl_sources TARGET fileset_sycl PROPERTY TYPE)
if(NOT type STREQUAL "SOURCES")
  message(SEND_ERROR "Expected TYPE SOURCES, got: ${type}")
endif()
get_property(language FILE_SET sycl_sources TARGET fileset_sycl
  PROPERTY LANGUAGE)
if(NOT language STREQUAL "SYCL")
  message(SEND_ERROR "Expected LANGUAGE SYCL, got: ${language}")
endif()
get_source_file_property(source_language FileSetSource.cxx LANGUAGE)
if(NOT source_language STREQUAL "CXX")
  message(SEND_ERROR "Source LANGUAGE changed to: ${source_language}")
endif()

add_library(fileset_default INTERFACE)
target_sources(fileset_default INTERFACE FILE_SET SYCL)
get_property(default_type FILE_SET SYCL TARGET fileset_default PROPERTY TYPE)
get_property(default_language FILE_SET SYCL TARGET fileset_default
  PROPERTY LANGUAGE)
if(NOT default_type STREQUAL "SOURCES" OR
   NOT default_language STREQUAL "SYCL")
  message(SEND_ERROR
    "Expected default SYCL set to have TYPE SOURCES and LANGUAGE SYCL")
endif()

add_library(fileset_canonical INTERFACE)
target_sources(fileset_canonical INTERFACE
  FILE_SET SYCL TYPE SOURCES FILES FileSetInherited.cxx)
get_property(canonical_type FILE_SET SYCL TARGET fileset_canonical
  PROPERTY TYPE)
get_property(canonical_language FILE_SET SYCL TARGET fileset_canonical
  PROPERTY LANGUAGE)
if(NOT canonical_type STREQUAL "SOURCES" OR
   NOT canonical_language STREQUAL "SYCL")
  message(SEND_ERROR
    "Expected canonical SYCL set to have TYPE SOURCES and LANGUAGE SYCL")
endif()
add_library(fileset_canonical_consumer OBJECT FileSetConsumer.cxx)
target_link_libraries(fileset_canonical_consumer PRIVATE fileset_canonical)
target_compile_definitions(fileset_canonical_consumer PRIVATE
  EXPECT_SYCL
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

add_library(fileset_manual OBJECT)
target_sources(fileset_manual PRIVATE
  FILE_SET manual_sources TYPE SOURCES FILES FileSetManual.cxx)
set_property(FILE_SET manual_sources TARGET fileset_manual
  PROPERTY LANGUAGE SYCL)
target_compile_definitions(fileset_manual PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

add_library(fileset_interface INTERFACE)
target_sources(fileset_interface INTERFACE
  FILE_SET inherited_sources TYPE SYCL FILES FileSetInherited.cxx)
add_library(fileset_consumer OBJECT FileSetConsumer.cxx)
target_link_libraries(fileset_consumer PRIVATE fileset_interface)
target_compile_definitions(fileset_consumer PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

set_source_files_properties(FileSetAsC.cxx PROPERTIES LANGUAGE CXX)
add_library(fileset_c OBJECT)
target_sources(fileset_c PRIVATE
  FILE_SET c_sources TYPE SOURCES FILES FileSetAsC.cxx)
set_property(FILE_SET c_sources TARGET fileset_c PROPERTY LANGUAGE C)
target_compile_definitions(fileset_c PRIVATE
  "$<$<COMPILE_LANGUAGE:C>:LANG_C>"
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>")
target_sources(fileset_c PRIVATE
  FILE_SET c_sycl_headers TYPE SYCL_HEADERS FILES FileSetViral.h)

add_library(fileset_interface_owner OBJECT FileSetSource.cxx)
target_sources(fileset_interface_owner INTERFACE
  FILE_SET owner_interface TYPE SYCL FILES FileSetSource.cxx)
target_compile_definitions(fileset_interface_owner PRIVATE
  EXPECT_CXX
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")
add_library(fileset_interface_owner_consumer OBJECT FileSetConsumer.cxx)
target_link_libraries(fileset_interface_owner_consumer
  PRIVATE fileset_interface_owner)
target_compile_definitions(fileset_interface_owner_consumer PRIVATE
  EXPECT_SYCL
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

add_library(fileset_header_interface INTERFACE)
target_sources(fileset_header_interface INTERFACE
  FILE_SET header_overlap_sources TYPE SYCL FILES FileSetHeaderOverlap.cxx)
add_library(fileset_header_consumer OBJECT FileSetConsumer.cxx)
target_sources(fileset_header_consumer PRIVATE
  FILE_SET HEADERS FILES FileSetHeaderOverlap.cxx)
target_link_libraries(fileset_header_consumer PRIVATE fileset_header_interface)
target_compile_definitions(fileset_header_consumer PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

add_library(fileset_head_leaf INTERFACE)
target_sources(fileset_head_leaf INTERFACE
  FILE_SET head_sources TYPE SYCL
  FILES "$<$<BOOL:$<TARGET_PROPERTY:USE_HEAD_SOURCE>>:${CMAKE_CURRENT_SOURCE_DIR}/FileSetHead.cxx>")
add_library(fileset_head_bridge INTERFACE)
target_link_libraries(fileset_head_bridge INTERFACE
  "$<$<BOOL:$<TARGET_PROPERTY:USE_HEAD_SOURCE>>:fileset_head_leaf>")
add_library(fileset_head_consumer OBJECT FileSetConsumer.cxx)
set_property(TARGET fileset_head_consumer PROPERTY USE_HEAD_SOURCE ON)
target_link_libraries(fileset_head_consumer PRIVATE fileset_head_bridge)
target_compile_definitions(fileset_head_consumer PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

add_library(fileset_sycl_headers_direct STATIC FileSetViral.cxx)
target_sources(fileset_sycl_headers_direct PRIVATE
  FILE_SET direct_sycl_headers TYPE HEADERS FILES FileSetViral.h)
set_property(FILE_SET direct_sycl_headers TARGET fileset_sycl_headers_direct
  PROPERTY LANGUAGE SYCL)
target_compile_definitions(fileset_sycl_headers_direct PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

set_source_files_properties(FileSetViralMain.cxx PROPERTIES LANGUAGE CXX)
add_executable(fileset_sycl_headers_exe FileSetViralMain.cxx)
target_sources(fileset_sycl_headers_exe PRIVATE
  FILE_SET SYCL_HEADERS FILES FileSetViral.h)
target_compile_definitions(fileset_sycl_headers_exe PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")
get_source_file_property(viral_source_language FileSetViralMain.cxx LANGUAGE)
if(NOT viral_source_language STREQUAL "CXX")
  message(SEND_ERROR
    "Viral SYCL headers changed source LANGUAGE to: ${viral_source_language}")
endif()

add_library(fileset_sycl_headers_private_consumer OBJECT FileSetSource.cxx)
target_link_libraries(fileset_sycl_headers_private_consumer
  PRIVATE fileset_sycl_headers_direct)
target_compile_definitions(fileset_sycl_headers_private_consumer PRIVATE
  EXPECT_CXX
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

add_library(fileset_sycl_headers_leaf INTERFACE)
target_sources(fileset_sycl_headers_leaf INTERFACE
  FILE_SET SYCL_HEADERS TYPE HEADERS FILES FileSetViral.h)
get_property(sycl_headers_type FILE_SET SYCL_HEADERS
  TARGET fileset_sycl_headers_leaf PROPERTY TYPE)
get_property(sycl_headers_language FILE_SET SYCL_HEADERS
  TARGET fileset_sycl_headers_leaf PROPERTY LANGUAGE)
if(NOT sycl_headers_type STREQUAL "HEADERS" OR
   NOT sycl_headers_language STREQUAL "SYCL")
  message(SEND_ERROR
    "Expected SYCL_HEADERS set to have TYPE HEADERS and LANGUAGE SYCL")
endif()
add_library(fileset_sycl_headers_bridge INTERFACE)
target_link_libraries(fileset_sycl_headers_bridge INTERFACE
  "$<$<BOOL:$<TARGET_PROPERTY:USE_SYCL_HEADERS>>:fileset_sycl_headers_leaf>")
add_library(fileset_sycl_headers_consumer OBJECT FileSetViral.cxx)
set_property(TARGET fileset_sycl_headers_consumer
  PROPERTY USE_SYCL_HEADERS ON)
target_link_libraries(fileset_sycl_headers_consumer
  PRIVATE fileset_sycl_headers_bridge)
target_compile_definitions(fileset_sycl_headers_consumer PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
  "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")
