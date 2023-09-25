cmake_policy(PUSH)
cmake_policy(SET CMP0057 NEW)

function (json_placeholders in out)
  string(REPLACE "<CONFIG>" "${CXXModules_config}" in "${in}")
  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    string(REPLACE "<CONFIG_DIR>" "/${CXXModules_config}" in "${in}")
  else ()
    string(REPLACE "<CONFIG_DIR>" "" in "${in}")
  endif ()
  if (CMAKE_BUILD_TYPE)
    string(REPLACE "<CONFIG_FORCE>" "${CXXModules_config}" in "${in}")
  else ()
    string(REPLACE "<CONFIG_FORCE>" "noconfig" in "${in}")
  endif ()
  string(REPLACE "<SOURCE_DIR>" "${RunCMake_SOURCE_DIR}" in "${in}")
  string(REPLACE "<BINARY_DIR>" "${RunCMake_TEST_BINARY_DIR}" in "${in}")
  string(REPLACE "<OBJEXT>" "${CMAKE_CXX_OUTPUT_EXTENSION}" in "${in}")
  set("${out}" "${in}" PARENT_SCOPE)
endfunction ()

function (check_json_value path actual_type expect_type actual_value expect_value)
  if (NOT actual_type STREQUAL expect_type)
    list(APPEND RunCMake_TEST_FAILED
      "Type mismatch at ${path}: ${actual_type} vs. ${expect_type}")
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
    return ()
  endif ()

  if (actual_type STREQUAL NULL)
    # Nothing to check
  elseif (actual_type STREQUAL BOOLEAN)
    if (NOT actual_value STREQUAL expect_value)
      list(APPEND RunCMake_TEST_FAILED
        "Boolean mismatch at ${path}: ${actual_value} vs. ${expect_value}")
    endif ()
  elseif (actual_type STREQUAL NUMBER)
    if (NOT actual_value EQUAL expect_value)
      list(APPEND RunCMake_TEST_FAILED
        "Number mismatch at ${path}: ${actual_value} vs. ${expect_value}")
    endif ()
  elseif (actual_type STREQUAL STRING)
    # Allow some values to be ignored.
    if (expect_value STREQUAL "<IGNORE>")
      return ()
    endif ()

    json_placeholders("${expect_value}" expect_value_expanded)
    if (NOT actual_value STREQUAL expect_value_expanded)
      list(APPEND RunCMake_TEST_FAILED
        "String mismatch at ${path}: ${actual_value} vs. ${expect_value_expanded}")
    endif ()
  elseif (actual_type STREQUAL ARRAY)
    check_json_array("${path}" "${actual_value}" "${expect_value}")
  elseif (actual_type STREQUAL OBJECT)
    check_json_object("${path}" "${actual_value}" "${expect_value}")
  endif ()

  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endfunction ()

# Check that two arrays are the same.
function (check_json_array path actual expect)
  string(JSON actual_len LENGTH "${actual}")
  string(JSON expect_len LENGTH "${expect}")

  set(iter_len "${actual_len}")
  if (actual_len LESS expect_len)
    list(APPEND RunCMake_TEST_FAILED
      "Missing array items at ${path}")
  elseif (expect_len LESS actual_len)
    list(APPEND RunCMake_TEST_FAILED
      "Extra array items at ${path}")
    set(iter_len "${expect_len}")
  endif ()

  foreach (idx RANGE "${iter_len}")
    if (idx EQUAL iter_len)
      break ()
    endif ()

    set(new_path "${path}[${idx}]")
    string(JSON actual_type TYPE "${actual}" "${idx}")
    string(JSON expect_type TYPE "${expect}" "${idx}")
    string(JSON actual_value GET "${actual}" "${idx}")
    string(JSON expect_value GET "${expect}" "${idx}")
    check_json_value("${new_path}" "${actual_type}" "${expect_type}" "${actual_value}" "${expect_value}")
  endforeach ()

  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endfunction ()

# Check that two inner objects are the same.
function (check_json_object path actual expect)
  string(JSON actual_len LENGTH "${actual}")
  string(JSON expect_len LENGTH "${expect}")

  set(actual_keys "")
  set(expect_keys "")
  foreach (idx RANGE "${actual_len}")
    if (idx EQUAL actual_len)
      break ()
    endif ()

    string(JSON actual_key MEMBER "${actual}" "${idx}")
    list(APPEND actual_keys "${actual_key}")
  endforeach ()
  foreach (idx RANGE "${expect_len}")
    if (idx EQUAL expect_len)
      break ()
    endif ()

    string(JSON expect_key MEMBER "${expect}" "${idx}")
    list(APPEND expect_keys "${expect_key}")
  endforeach ()

  json_placeholders("${expect_keys}" expect_keys_expanded)

  set(actual_keys_missed "${actual_keys}")
  set(expect_keys_missed "${expect_keys}")

  set(common_keys "")
  set(expect_keys_stack "${expect_keys}")
  while (expect_keys_stack)
    list(POP_BACK expect_keys_stack expect_key)
    json_placeholders("${expect_key}" expect_key_expanded)

    if (expect_key_expanded IN_LIST actual_keys_missed AND
        expect_key IN_LIST expect_keys_missed)
      list(APPEND common_keys "${expect_key}")
    endif ()

    list(REMOVE_ITEM actual_keys_missed "${expect_key_expanded}")
    list(REMOVE_ITEM expect_keys_missed "${expect_key}")
  endwhile ()

  if (actual_keys_missed)
    string(REPLACE ";" ", " actual_keys_missed_text "${actual_keys_missed}")
    list(APPEND RunCMake_TEST_FAILED
      "Extra unexpected members at ${path}: ${actual_keys_missed_text}")
  endif ()
  if (expect_keys_missed)
    string(REPLACE ";" ", " expect_keys_missed_text "${expect_keys_missed}")
    list(APPEND RunCMake_TEST_FAILED
      "Missing expected members at ${path}: ${expect_keys_missed_text}")
  endif ()

  foreach (key IN LISTS common_keys)
    json_placeholders("${key}" key_expanded)
    set(new_path "${path}.${key_expanded}")
    string(JSON actual_type TYPE "${actual}" "${key_expanded}")
    string(JSON expect_type TYPE "${expect}" "${key}")
    string(JSON actual_value GET "${actual}" "${key_expanded}")
    string(JSON expect_value GET "${expect}" "${key}")
    check_json_value("${new_path}" "${actual_type}" "${expect_type}" "${actual_value}" "${expect_value}")
  endforeach ()

  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endfunction ()

# Check that two JSON objects are the same.
function (check_json actual expect)
  check_json_object("" "${actual}" "${expect}")

  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endfunction ()

cmake_policy(POP)
