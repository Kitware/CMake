foreach(c IN LISTS ComponentTest_FIND_COMPONENTS)
  message(STATUS
    "ComponentTest_FIND_REQUIRED_${c} "
    "'${ComponentTest_FIND_REQUIRED_${c}}'")
endforeach()
