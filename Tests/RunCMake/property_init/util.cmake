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
