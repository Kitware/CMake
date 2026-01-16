include(RunCMake)

cmake_policy(SET CMP0140 NEW)

# Arguments after the first are the files to validate against the schema
function(validate_fileapi_schema schema)
  if(NOT ARGN)
    # No files to validate against the schema
    return()
  endif()
  list(JOIN ARGN "\n" file_list)
  set(file_list_file ${RunCMake_TEST_BINARY_DIR}/check_file_list.txt)
  file(WRITE "${file_list_file}" "${file_list}")
  execute_process(
    COMMAND ${Python_EXECUTABLE}
      "${RunCMake_SOURCE_DIR}/fileapi_validate_schema.py"
        "${file_list_file}"
        "${schema}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
  )
  if(NOT result STREQUAL 0)
    string(REPLACE "\n" "\n  " output "${output}")
    string(APPEND RunCMake_TEST_FAILED
      "Failed to validate files against JSON schema: ${schema}\nOutput:\n${output}\n")
  endif()
  return(PROPAGATE RunCMake_TEST_FAILED)
endfunction()

# Function called in *-check.cmake scripts to check api files.
function(check_api expect)
  file(GLOB_RECURSE actual
    LIST_DIRECTORIES TRUE
    RELATIVE ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1
    ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/*
    )
  if(NOT "${actual}" MATCHES "${expect}")
    set(RunCMake_TEST_FAILED "API files:
  ${actual}
do not match what we expected:
  ${expect}
in directory:
  ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1")
  endif()
  if(NOT RunCMake_TEST_FAILED AND Python_EXECUTABLE AND CMake_TEST_JSON_SCHEMA)
    cmake_path(SET schema_dir NORMALIZE ${RunCMake_SOURCE_DIR}/../../../Help/manual/file_api)
    set(prefix ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1)
    set(replies ${actual})
    list(FILTER replies INCLUDE REGEX "^reply/")
    set(schema_types
      index   # Special case, error replies also use this schema
      codemodel
      directory
      target
      configureLog
      cache
      cmakeFiles
      toolchains
    )
    foreach(schema_type IN LISTS schema_types)
      set(schema_type_${schema_type} "")
    endforeach()
    foreach(file IN LISTS replies)
      if(file MATCHES "^reply/(index|error)-")
        list(APPEND schema_type_index ${prefix}/${file})
      else()
        foreach(schema_type IN LISTS schema_types)
          if(file MATCHES "^reply/${schema_type}-")
            list(APPEND schema_type_${schema_type} ${prefix}/${file})
          endif()
        endforeach()
      endif()
    endforeach()
    foreach(schema_type IN LISTS schema_types)
      validate_fileapi_schema(
        ${schema_dir}/schema_${schema_type}.json
        ${schema_type_${schema_type}}
      )
    endforeach()
  endif()
  return(PROPAGATE RunCMake_TEST_FAILED)
endfunction()

function(check_stateful_queries)
  if(RunCMake_TEST_FAILED OR NOT Python_EXECUTABLE OR NOT CMake_TEST_JSON_SCHEMA)
    return()
  endif()

  cmake_path(SET schema_dir NORMALIZE
    ${RunCMake_SOURCE_DIR}/../../../Help/manual/file_api
  )
  set(prefix ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client)
  list(TRANSFORM ARGN
    REPLACE "^(.+)$" "${prefix}-\\1/query.json"
    OUTPUT_VARIABLE query_json_files
  )
  validate_fileapi_schema(
    ${schema_dir}/schema_stateful_query.json
    ${query_json_files}
  )
  return(PROPAGATE RunCMake_TEST_FAILED)
endfunction()

function(check_python case prefix)
  if(RunCMake_TEST_FAILED OR NOT Python_EXECUTABLE)
    return()
  endif()
  file(GLOB index ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/reply/${prefix}-*.json)
  execute_process(
    COMMAND ${Python_EXECUTABLE} "${RunCMake_SOURCE_DIR}/${case}-check.py"
      --build-dir "${RunCMake_TEST_BINARY_DIR}"
      --reply-index "${index}"
      --cxx-compiler-id "${CMAKE_CXX_COMPILER_ID}"
      --cxx-simulate-id "${CMAKE_CXX_SIMULATE_ID}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    )
  if(NOT result EQUAL 0)
    string(REPLACE "\n" "\n  " output "  ${output}")
    set(RunCMake_TEST_FAILED "Unexpected index:\n${output}" PARENT_SCOPE)
  endif()
endfunction()

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_CONFIGURATION_TYPES=Debug\\;Release\\;MinSizeRel\\;RelWithDebInfo")
endif()

if(JsonCpp_VERSION AND JsonCpp_VERSION VERSION_LESS 1.7.5)
  set(ENV{CMake_JSONCPP_PRE_1_7_5} 1)
endif()

run_cmake(Nothing)
run_cmake(Empty)
run_cmake(EmptyClient)
run_cmake(Stale)
run_cmake(SharedStateless)
run_cmake(ClientStateless)
run_cmake(MixedStateless)
run_cmake(DuplicateStateless)
run_cmake(ClientStateful)
run_cmake_with_options(InitialCache -C ${RunCMake_SOURCE_DIR}/InitialCache-script.cmake)
run_cmake(ProjectQueryGood)
run_cmake(ProjectQueryBad)
run_cmake(FailConfigure)

function(run_object object)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${object}-build)
  list(APPEND RunCMake_TEST_OPTIONS ${ARGN} -DCMAKE_POLICY_DEFAULT_CMP0118=NEW)
  run_cmake(${object})
  list(POP_BACK RunCMake_TEST_OPTIONS)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${object}-SharedStateless ${CMAKE_COMMAND} .)
  run_cmake_command(${object}-ClientStateless ${CMAKE_COMMAND} .)
  run_cmake_command(${object}-ClientStateful ${CMAKE_COMMAND} .)
  run_cmake_command(${object}-FailConfigure ${CMAKE_COMMAND} . -DFAIL=1)
endfunction()

run_object(codemodel-v2)
run_object(configureLog-v1)
run_object(cache-v2)
run_object(cmakeFiles-v1)
run_object(toolchains-v1)
run_object(toolchains-v1 -DTOOLCHAINSV1_COMPILERARGS=1)
run_object(toolchains-v1 -DTOOLCHAINSV1_COMPILERARGS=2)
