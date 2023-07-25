set(all_target_types
  "EXECUTABLE"

  "IMPORTED_EXECUTABLE"

  "INTERFACE"
  "MODULE"
  "OBJECT"
  "SHARED"
  "STATIC"

  "IMPORTED_INTERFACE"
  "IMPORTED_MODULE"
  "IMPORTED_OBJECT"
  "IMPORTED_SHARED"
  "IMPORTED_STATIC"

  "CUSTOM")

function (prepare_target_types name)
  set("${name}" "${ARGN}" PARENT_SCOPE)
  list(REMOVE_ITEM all_target_types ${ARGN})
  set("not_${name}" "${all_target_types}" PARENT_SCOPE)
endfunction ()

function (per_config variable)
  prepare_properties("${property_table}" properties expected_values expected_alias)

  get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if (is_multi_config)
    set(configs "${CMAKE_CONFIGURATION_TYPES}")
  else ()
    if (NOT CMAKE_BUILD_TYPE STREQUAL "")
      set(configs "${CMAKE_BUILD_TYPE}")
    endif ()
  endif ()

  foreach (property expected alias IN ZIP_LISTS expected_properties expected_values expected_alias)
    if (property MATCHES "^_")
      set(prepend 1)
    elseif (property MATCHES "_$")
      set(prepend 0)
    else ()
      message(SEND_ERROR
        "Per-config properties must have a `_` at one end of their name: '${property}'")
    endif ()
    foreach (config IN LISTS configs)
      if (prepend)
        list(APPEND "${variable}"
          "${config}_${property}" "${value}/${config}" "${alias}")
      else ()
        list(APPEND "${variable}"
          "${property}_${config}" "${value}/${config}" "${alias}")
      endif ()
    endforeach ()
  endforeach ()

  set("${variable}" "${${variable}}" PARENT_SCOPE)
endfunction ()

function (make_target name type)
  if (type STREQUAL "EXECUTABLE")
    add_executable("${name}")
    target_sources("${name}" PRIVATE ${main_sources})
  elseif (type STREQUAL "IMPORTED_EXECUTABLE")
    add_executable("${name}" IMPORTED)
    set_property(TARGET "${name}" PROPERTY IMPORTED_LOCATION "${CMAKE_COMMAND}")
  elseif (type STREQUAL "CUSTOM")
    add_custom_target("${name}" COMMAND "${CMAKE_EXECUTABLE}" -E echo "${name}")
  elseif (type MATCHES "IMPORTED_")
    string(REPLACE "IMPORTED_" "" type "${type}")
    add_library("${name}" IMPORTED ${type})
    if (NOT type STREQUAL "INTERFACE")
      set_property(TARGET "${name}" PROPERTY IMPORTED_LOCATION "${default_library_location}")
    endif ()
  else ()
    add_library("${name}" ${type})
    target_sources("${name}" PRIVATE ${library_sources})
  endif ()

  if (type MATCHES "EXECUTABLE")
    add_executable("alias::${name}" ALIAS "${name}")
  elseif (NOT type STREQUAL "CUSTOM")
    add_library("alias::${name}" ALIAS "${name}")
  endif ()
endfunction ()

function (check_property target property expected)
  if (NOT TARGET "${target}")
    message(SEND_ERROR
      "No such target '${target}'")
    return ()
  endif ()

  get_property(is_set TARGET "${target}" PROPERTY "${property}" SET)
  if (is_set)
    get_property(actual TARGET "${target}" PROPERTY "${property}")
  endif ()
  if (expected STREQUAL "<UNSET>")
    if (is_set)
      message(SEND_ERROR
        "Target '${target}' should not have '${property}' set at all, but is '${actual}'")
    endif ()
  elseif (is_set AND NOT expected STREQUAL actual)
    message(SEND_ERROR
      "Target '${target}' should have '${property}' set to '${expected}', but is '${actual}'")
  elseif (NOT is_set)
    message(SEND_ERROR
      "Target '${target}' should have '${property}' set to '${expected}', but is not set at all")
  endif ()
endfunction ()

function (prepare_properties table output_properties output_expected output_alias)
  set(_properties)
  set(_expected)
  set(_alias)

  set(variable "_properties")
  foreach (item IN LISTS "${table}")
    list(APPEND "${variable}" "${item}")
    if (variable STREQUAL "_properties")
      set(variable "_expected")
    elseif (variable STREQUAL "_expected")
      set(variable "_alias")
    elseif (variable STREQUAL "_alias")
      set(variable "_properties")
    else ()
      message(FATAL_ERROR
        "Failed to track property table parsing")
    endif ()
  endforeach ()
  if (NOT variable STREQUAL "_properties")
    message(FATAL_ERROR
      "Table does not have a multiple of 3 items")
  endif ()

  set("${output_properties}" "${_properties}" PARENT_SCOPE)
  set("${output_expected}" "${_expected}" PARENT_SCOPE)
  set("${output_alias}" "${_alias}" PARENT_SCOPE)
endfunction ()

# Contextual variables:
#   iteration: make unique target names
#   with_defaults: if set, do not set variables, but instead test internal
#                  default calculations
function (run_property_tests applied_types property_table)
  prepare_properties("${property_table}" expected_properties expected_values expected_alias)

  if (NOT with_defaults)
    foreach (property expected IN ZIP_LISTS expected_properties expected_values)
      string(REPLACE "<SEMI>" ";" expected "${expected}")
      set("CMAKE_${property}" "${expected}")
    endforeach ()
  endif ()

  foreach (target_type IN LISTS "${applied_types}")
    set(target_name "${RunCMake_TEST}${iteration}-${target_type}")
    if (with_defaults)
      string(APPEND target_name "-defaults")
    endif ()
    make_target("${target_name}" "${target_type}")
    foreach (property expected alias IN ZIP_LISTS expected_properties expected_values expected_alias)
      string(REPLACE "<SEMI>" ";" expected "${expected}")
      check_property("${target_name}" "${property}" "${expected}")
      if (NOT target_type STREQUAL "CUSTOM")
        if (alias STREQUAL "<SAME>")
          check_property("alias::${target_name}" "${property}" "${expected}")
        elseif (alias STREQUAL "<UNSET>")
          check_property("alias::${target_name}" "${property}" "<UNSET>")
        else ()
          message(FATAL_ERROR
            "Invalid `alias` entry for property '${property}': '${alias}'")
        endif ()
      endif ()
    endforeach ()
  endforeach ()

  foreach (target_type IN LISTS "not_${applied_types}")
    set(target_name "${RunCMake_TEST}${iteration}-${target_type}-unset")
    if (with_defaults)
      string(APPEND target_name "-defaults")
    endif ()
    make_target("${target_name}" "${target_type}")
    foreach (property IN LISTS expected_properties)
      check_property("${target_name}" "${property}" "<UNSET>")
      if (NOT target_type STREQUAL "CUSTOM")
        check_property("alias::${target_name}" "${property}" "<UNSET>")
      endif ()
    endforeach ()
  endforeach ()
endfunction ()
