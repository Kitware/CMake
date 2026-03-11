include("${VARS_FILE}")

list(LENGTH ACTUAL al)
list(LENGTH EXPECTED el)
if(NOT al EQUAL el)
  message(FATAL_ERROR "Got the wrong number of objects")
endif()

foreach(a e IN ZIP_LISTS ACTUAL EXPECTED)
  if(NOT a MATCHES "${e}")
    message(SEND_ERROR "Object \"${a}\" does not match expected regex \"${e}\"")
  endif()
endforeach()
