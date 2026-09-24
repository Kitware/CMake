set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
enable_language(CXX)
enable_language(SYCL)

set(cmake_version_save "${CMAKE_VERSION}")
# The experimental file-set export format currently requires CMake 4.5.
set(CMAKE_VERSION 4.5.0)
cmake_path(GET CMAKE_BINARY_DIR PARENT_PATH tests_dir)
include("${tests_dir}/export/export.cmake")
include("${tests_dir}/export/install/lib/cmake/install-export.cmake")
set(CMAKE_VERSION "${cmake_version_save}")

function(check_file_set target file_set expected_type expected_language)
  get_property(type FILE_SET "${file_set}" TARGET "${target}" PROPERTY TYPE)
  if(NOT type STREQUAL expected_type)
    message(SEND_ERROR
      "Expected ${target} ${file_set} TYPE ${expected_type}, got: ${type}")
  endif()
  get_property(language FILE_SET "${file_set}" TARGET "${target}"
    PROPERTY LANGUAGE)
  if(NOT language STREQUAL expected_language)
    message(SEND_ERROR
      "Expected ${target} ${file_set} LANGUAGE ${expected_language}, "
      "got: ${language}")
  endif()
endfunction()

foreach(namespace IN ITEMS export install)
  set(imported_target "${namespace}::fileset_export")
  check_file_set("${imported_target}" export_sources SOURCES SYCL)
  check_file_set("${imported_target}" SYCL SOURCES SYCL)

  add_library("fileset_${namespace}_consumer" OBJECT FileSetConsumer.cxx)
  target_link_libraries("fileset_${namespace}_consumer"
    PRIVATE "${imported_target}")
  target_compile_definitions("fileset_${namespace}_consumer" PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
    "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

  set(override_target "${namespace}::fileset_default_override_export")
  check_file_set("${override_target}" SYCL SOURCES CXX)
  add_library("fileset_${namespace}_override_consumer" OBJECT
    FileSetConsumer.cxx)
  target_link_libraries("fileset_${namespace}_override_consumer"
    PRIVATE "${override_target}")
  target_compile_definitions("fileset_${namespace}_override_consumer" PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
    "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")

  set(headers_target "${namespace}::fileset_headers_export")
  check_file_set("${headers_target}" SYCL_HEADERS HEADERS SYCL)
  add_library("fileset_${namespace}_headers_consumer" OBJECT FileSetViral.cxx)
  target_link_libraries("fileset_${namespace}_headers_consumer"
    PRIVATE "${headers_target}")
  target_compile_definitions("fileset_${namespace}_headers_consumer" PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:LANG_CXX>"
    "$<$<COMPILE_LANGUAGE:SYCL>:LANG_SYCL>")
endforeach()
