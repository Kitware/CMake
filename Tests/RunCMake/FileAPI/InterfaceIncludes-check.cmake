set(expected_json [=[[
  {"path": "all-languages"},
  {"path": "c-only", "languages": ["C"]},
  {"path": "common"},
  {"path": "cxx-id", "languages": ["CXX"]},
  {"path": "cxx-only", "languages": ["CXX"]},
  {"path": "lang-C", "languages": ["C"]},
  {"path": "lang-CXX", "languages": ["CXX"]},
  {"path": "mixed", "languages": ["C"]},
  {"path": "mixed", "isSystem": true, "languages": ["CXX"]},
  {"path": "system-C", "isSystem": true, "languages": ["C"]},
  {"path": "system-CXX", "isSystem": true, "languages": ["CXX"]},
  {"path": "system-common", "isSystem": true},
  {"path": "system-cxx-id", "isSystem": true, "languages": ["CXX"]}
]]=])
file(READ "${RunCMake_TEST_BINARY_DIR}/enabled-languages.txt" enabled_languages)
set(extra_languages ${enabled_languages})
list(REMOVE_ITEM extra_languages C CXX)
set(non_cxx_languages ${enabled_languages})
list(REMOVE_ITEM non_cxx_languages CXX)
list(SORT non_cxx_languages)
string(REPLACE ";" "\", \"" non_cxx_languages "${non_cxx_languages}")
foreach(language IN LISTS extra_languages)
  string(JSON index LENGTH "${expected_json}")
  string(JSON expected_json SET "${expected_json}" ${index}
    "{\"path\": \"lang-${language}\", \"languages\": [\"${language}\"]}")
endforeach()
string(JSON expected_count LENGTH "${expected_json}")
math(EXPR last "${expected_count} - 1")
set(expected_entries)
foreach(index RANGE 0 ${last})
  string(JSON entry GET "${expected_json}" ${index})
  string(JSON path GET "${entry}" path)
  if(path STREQUAL "all-languages" AND extra_languages)
    string(JSON entry SET "${entry}" languages "[\"C\", \"CXX\"]")
  elseif(path STREQUAL "mixed")
    string(JSON is_system ERROR_VARIABLE error GET "${entry}" isSystem)
    if(error)
      string(JSON entry SET "${entry}" languages "[\"${non_cxx_languages}\"]")
    endif()
  endif()
  string(JSON expected_json SET "${expected_json}" ${index} "${entry}")
  list(APPEND expected_entries "${entry}")
endforeach()
foreach(target IN ITEMS concrete headers imported)
  file(GLOB replies "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/reply/target-${target}-*.json")
  if(NOT replies)
    set(RunCMake_TEST_FAILED
      "No File API reply for ${target} in ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/reply\nEnabled languages: ${enabled_languages}")
    return()
  endif()
  validate_json_schema("${RunCMake_SOURCE_DIR}/../../../Help/manual/file_api/schema_target.json" "${replies}")
  foreach(reply IN LISTS replies)
    file(READ "${reply}" contents)
    string(JSON actual_json ERROR_VARIABLE json_error GET "${contents}" interfaceIncludes)
    if(json_error)
      set(actual_json "Unable to read interfaceIncludes: ${json_error}")
    endif()
    set(diagnostics
      "Enabled languages: ${enabled_languages}\nExpected interfaceIncludes (paths relative to ${RunCMake_TEST_BINARY_DIR}/include):\n${expected_json}\nActual interfaceIncludes:\n${actual_json}")
    string(JSON count ERROR_VARIABLE error LENGTH "${contents}" interfaceIncludes)
    if(error OR NOT count EQUAL expected_count)
      set(RunCMake_TEST_FAILED "Expected ${expected_count} interface include entries in ${reply}, got ${count}\n${diagnostics}")
      return()
    endif()
    set(remaining_entries "${expected_entries}")
    foreach(index RANGE 0 ${last})
      string(JSON path GET "${contents}" interfaceIncludes ${index} path)
      cmake_path(GET path FILENAME dir)
      if(NOT path STREQUAL "${RunCMake_TEST_BINARY_DIR}/include/${dir}")
        set(RunCMake_TEST_FAILED "Unexpected interface include path at index ${index} in ${reply}: ${path}\n${diagnostics}")
        return()
      endif()
      string(JSON entry GET "${contents}" interfaceIncludes ${index})
      string(JSON entry SET "${entry}" path "\"${dir}\"")
      list(FIND remaining_entries "${entry}" match)
      if(match EQUAL -1)
        set(RunCMake_TEST_FAILED "Unexpected or duplicate interface include entry at index ${index} in ${reply}:\n${entry}\n${diagnostics}")
        return()
      endif()
      list(REMOVE_AT remaining_entries ${match})
    endforeach()
  endforeach()
endforeach()
