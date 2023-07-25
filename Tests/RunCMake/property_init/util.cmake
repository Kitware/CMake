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
