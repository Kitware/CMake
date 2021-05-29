foreach(flag @flags@)
  string(FIND "${actual_stdout}" "${flag}" position)

  if(NOT position EQUAL -1)
    set(found TRUE)
    break()
  endif()
endforeach()

if(NOT found)
  set(RunCMake_TEST_FAILED "No compile flags from \"@flags@\" found for CMAKE_@lang@_EXTENSIONS=@extensions_opposite@.")
endif()
