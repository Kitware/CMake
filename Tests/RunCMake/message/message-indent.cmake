function(debug_list LIST_VAR)
  message(STATUS "${LIST_VAR}:")
  list(APPEND CMAKE_MESSAGE_INDENT "   ")
  foreach(_item IN LISTS ${LIST_VAR})
    list(LENGTH ${_item} _item_len)
    if(_item_len GREATER 1)
      debug_list(${_item})
    else()
      message(STATUS "${_item}")
    endif()
  endforeach()
endfunction()

list(APPEND COUNTING_ENGLISH one two three four five)
list(APPEND COUNTING_BAHASA satu dua tiga empat lima)

list(APPEND COUNTING COUNTING_ENGLISH COUNTING_BAHASA)

debug_list(COUNTING)
