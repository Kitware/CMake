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
