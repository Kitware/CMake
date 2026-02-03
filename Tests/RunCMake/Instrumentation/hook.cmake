cmake_minimum_required(VERSION 3.30)

include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/verify-snippet.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/verify-trace.cmake)

# Test CALLBACK script. Prints output information and verifies index file
# Called as: cmake -P hook.cmake [CheckForStaticQuery?] [CheckForTrace?] [index.json]
set(index ${CMAKE_ARGV5})
if (NOT ${CMAKE_ARGV3})
  set(hasStaticInfo "UNEXPECTED")
endif()
if (NOT ${CMAKE_ARGV4})
  set(hasTrace "UNEXPECTED")
endif()
read_json("${index}" contents)
string(JSON hook GET "${contents}" hook)

# Output is verified by *-stdout.txt files that the HOOK is run
message(STATUS ${hook})
# Not a check-*.cmake script, this is called as an instrumentation CALLBACK
set(ERROR_MESSAGE "")
function(add_error error)
  string(APPEND ERROR_MESSAGE "${error}\n")
  return(PROPAGATE ERROR_MESSAGE)
endfunction()

json_has_key("${index}" "${contents}" version)
json_has_key("${index}" "${contents}" buildDir)
json_has_key("${index}" "${contents}" dataDir)
json_has_key("${index}" "${contents}" snippets)

if (NOT version EQUAL 1)
  add_error("Version must be 1, got: ${version}")
endif()

string(JSON n_snippets LENGTH "${snippets}")

math(EXPR snippets_range "${n_snippets}-1")
foreach(i RANGE ${snippets_range})
  string(JSON filename GET "${snippets}" ${i})
  if (NOT EXISTS ${dataDir}/${filename})
    add_error("Listed snippet: ${dataDir}/${filename} does not exist")
  endif()
  read_json(${dataDir}/${filename} snippet_contents)
  verify_snippet_file(${dataDir}/${filename} "${snippet_contents}")
endforeach()

json_has_key("${index}" "${contents}" trace ${hasTrace})
if (NOT hasTrace STREQUAL UNEXPECTED)
  if (NOT EXISTS ${dataDir}/${trace})
    add_error("Listed trace file: ${dataDir}/${trace} does not exist")
  endif()
  verify_trace_file_name("${index}" "${trace}")
  read_json(${dataDir}/${trace} trace_contents)
  string(JSON n_entries LENGTH "${trace_contents}")
  if (n_entries EQUAL 0)
    add_error("Listed trace file: ${dataDir}/${trace} has no entries")
  endif()
  if (NOT n_entries EQUAL n_snippets)
    add_error("Differing number of trace entries (${n_entries}) and snippets (${n_snippets})")
  endif()

  math(EXPR entries_range "${n_entries}-1")
  foreach (i RANGE ${entries_range})
    string(JSON entry GET "${trace_contents}" ${i})
    verify_trace_entry("${trace}" "${entry}")

    # In addition to validating the data in the trace entry, check that
    # it is strictly equal to its corresponding snippet data.
    # Ideally, the args from all trace entries could be checked at once
    # against the list of snippets from the index file, but the order of
    # snippets is not preserved in the trace file, so being equal to data from
    # any snippet file is sufficient.
    set(args_equals_snippet OFF)
    string(JSON trace_args GET "${entry}" args)
    foreach (j RANGE ${entries_range})
      string(JSON snippet_file GET "${snippets}" ${j})
      read_json(${dataDir}/${snippet_file} snippet_contents)
      string(JSON args_equals_snippet EQUAL "${snippet_contents}" "${trace_args}")
      if (args_equals_snippet)
        break()
      endif()
    endforeach()
    if (NOT args_equals_snippet)
      add_error("Trace entry args does not match any snippet data: ${entry}")
    endif()
  endforeach()
endif()

json_has_key("${index}" "${contents}" staticSystemInformation ${hasStaticInfo})
if (NOT hasStaticInfo STREQUAL UNEXPECTED)
  json_has_key("${index}" "${staticSystemInformation}" OSName ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" OSPlatform ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" OSRelease ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" OSVersion ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" familyId ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" hostname ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" is64Bits ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" modelId ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" modelName ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" numberOfLogicalCPU ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" numberOfPhysicalCPU ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" processorAPICID ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" processorCacheSize ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" processorClockFrequency ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" processorName ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" totalPhysicalMemory ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" totalVirtualMemory ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" vendorID ${hasStaticInfo})
  json_has_key("${index}" "${staticSystemInformation}" vendorString ${hasStaticInfo})

  # FIXME(#27545): We currently do not guarantee that the fields above which
  # output strings are non-empty. `vendorString` and `processorName` especially
  # have shown issues on some platforms. This test is logically equivalent to
  # RunCMake.cmake_host_system_information, which uses the same underlying
  # implementation.
  set(string_fields
    OSName
    OSPlatform
    OSRelease
    OSVersion
    familyId
    hostname
    modelId
    modelName
    vendorID
    vendorString
  )
  foreach (field IN LISTS string_fields)
    string(JSON ${field}_type TYPE "${staticSystemInformation}" ${field})
    if (NOT "${${field}_type}" STREQUAL "NULL" AND NOT "${${field}_type}" STREQUAL "STRING")
      add_error("Got bad type '${${field}_type}' for field '${field}': ${${field}}")
    endif()
    if ("${${field}_type}" STREQUAL "STRING" AND ${field} STREQUAL "")
      add_error("Got empty string for field '${field}'")
    endif()
  endforeach()

  # We guarantee that the numeric fields are either indeed numeric, or else
  # null.
  set(numeric_fields
    numberOfLogicalCPU
    numberOfPhysicalCPU
    processorAPICID
    processorCacheSize
    processorClockFrequency
    totalPhysicalMemory
    totalVirtualMemory
  )
  foreach (field IN LISTS numeric_fields)
    string(JSON ${field}_type TYPE "${staticSystemInformation}" ${field})
    if (NOT "${${field}_type}" STREQUAL "NULL" AND NOT "${${field}_type}" STREQUAL "NUMBER")
      add_error("Got bad type '${${field}_type}' for field '${field}': ${${field}}")
    endif()
    if ("${${field}_type}" STREQUAL "NUMBER" AND ${field} LESS_EQUAL 0)
      add_error("Got bad value for field '${field}': ${${field}}")
    endif()
  endforeach()
endif()

get_filename_component(v1 ${dataDir} DIRECTORY)
if (EXISTS ${v1}/${hook}.hook)
  add_error("Received multiple triggers of the same hook: ${hook}")
endif()
file(WRITE ${v1}/${hook}.hook "${ERROR_MESSAGE}")

if (NOT ERROR_MESSAGE MATCHES "^$")
  message(FATAL_ERROR ${ERROR_MESSAGE})
endif()
