message(STATUS "=============================================================")
message(STATUS "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")
message(STATUS "")

if(NOT CPackIFWGenerator_BINARY_DIR)
  message(FATAL_ERROR "CPackIFWGenerator_BINARY_DIR not set")
endif()

message(STATUS "CMAKE_COMMAND: ${CMAKE_COMMAND}")
message(STATUS "CMAKE_CPACK_COMMAND: ${CMAKE_CPACK_COMMAND}")
message(STATUS "CPackIFWGenerator_BINARY_DIR: ${CPackIFWGenerator_BINARY_DIR}")

if(config)
  set(_C_config -C ${config})
endif()

execute_process(COMMAND "${CMAKE_CPACK_COMMAND}"
                        ${_C_config}
  RESULT_VARIABLE CPack_result
  OUTPUT_VARIABLE CPack_output
  ERROR_VARIABLE CPack_error
  WORKING_DIRECTORY "${CPackIFWGenerator_BINARY_DIR}")

if(CPack_result)
  message(FATAL_ERROR "CPack execution went wrong!, CPack_output=${CPack_output}, CPack_error=${CPack_error}")
else ()
  message(STATUS "CPack_output=${CPack_output}")
endif()

set(expected_file_mask "${CPackIFWGenerator_BINARY_DIR}/_CPack_Packages/*/IFW/*/config/config.xml")
file(GLOB project_file "${expected_file_mask}")

message(STATUS "project_file='${project_file}'")
message(STATUS "expected_file_mask='${expected_file_mask}'")

if(NOT project_file)
  message(FATAL_ERROR "project_file does not exist.")
endif()

# should match !define MUI_HEADERIMAGE_BITMAP "${PROJECT_SOURCE_DIR}\header-image.bmp"
file(STRINGS "${project_file}" lines)

file(STRINGS "${project_file}" tag_start REGEX [[<ProductImage>]])
file(STRINGS "${project_file}" tag_end REGEX [[</ProductImage>]])
list(LENGTH tag_start tag_start_size)
list(LENGTH tag_end tag_end_size)

if(NOT tag_start_size EQUAL 2)
  message(FATAL_ERROR "Expected to have 2 <ProductImage> not ${tag_start_size}")
endif()
if(NOT tag_end_size EQUAL 2)
  message(FATAL_ERROR "Expected to have 2 </ProductImage> not ${tag_end_size}")
endif()

file(STRINGS "${project_file}" tag_start REGEX [[<Image>]])
file(STRINGS "${project_file}" tag_end REGEX [[</Image>]])
list(LENGTH tag_start tag_start_size)
list(LENGTH tag_end tag_end_size)

if(NOT tag_start_size EQUAL 2)
  message(FATAL_ERROR "Expected to have 2 <Image> not ${tag_start_size}")
endif()
if(NOT tag_end_size EQUAL 2)
  message(FATAL_ERROR "Expected to have 2 </Image> not ${tag_end_size}")
endif()

file(STRINGS "${project_file}" tag_start REGEX [[<Url>]])
file(STRINGS "${project_file}" tag_end REGEX [[</Url>]])
list(LENGTH tag_start tag_start_size)
list(LENGTH tag_end tag_end_size)

if(NOT tag_start_size EQUAL 2)
  message(FATAL_ERROR "Expected to have 2 <Url> not ${tag_start_size}")
endif()
if(NOT tag_end_size EQUAL 2)
  message(FATAL_ERROR "Expected to have 2 </Url> not ${tag_end_size}")
endif()

set(TO_SEARCHES
  "<ProductImages>"
  "<Image>ApplicationIcon.png</Image>"
  "<Url>https://www.ApplicationIcon.org</Url>"
  "<Image>SplashScreen.png</Image>"
  "<Url>https://www.SplashScreen.org</Url>"
  "</ProductImages>"
)
foreach(TO_SEARCH ${TO_SEARCHES})
  string(FIND "${lines}" "${TO_SEARCH}" output_index)
  message(STATUS "Found the ${TO_SEARCH} at index ${output_index}")
  if("${output_index}" EQUAL "-1")
    message(FATAL_ERROR "${TO_SEARCH} not found in the project")
  endif()
endforeach()
