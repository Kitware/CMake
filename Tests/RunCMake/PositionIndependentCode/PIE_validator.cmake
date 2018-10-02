
include_guard(GLOBAL)

function (CHECK_EXECUTABLE executable result)
  set (${result} "UNKNOWN" PARENT_SCOPE)

  if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set (tool otool -hv)
  else()
    set (tool "${CMAKE_COMMAND}" -E env LANG=C LC_ALL=C readelf -lW)
  endif()

  execute_process(COMMAND ${tool} "${executable}"
                  OUTPUT_VARIABLE output
                  ERROR_VARIABLE output)

  if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if (output MATCHES "( |\t)PIE( |\n|$)")
      set (${result} "PIE" PARENT_SCOPE)
    else()
      set (${result} "NO_PIE" PARENT_SCOPE)
    endif()
  else()
    if (output MATCHES "Elf file type is DYN")
      set (${result} "PIE" PARENT_SCOPE)
    elseif (output MATCHES "Elf file type is EXEC")
      set (${result} "NO_PIE" PARENT_SCOPE)
    else()
      message(SEND_ERROR "Did not find a known file type")
    endif()
  endif()
endfunction()
