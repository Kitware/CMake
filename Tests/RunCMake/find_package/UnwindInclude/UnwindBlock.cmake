set(RunCMake_TEST_FAILED "Failed to observe side effects of block() scopes during unwind")

block()
  find_package(foo UNWIND_INCLUDE)
endblock()

block(PROPAGATE BLOCK_RUN PrimaryUnwind_FOUND)
  set(BLOCK_RUN true)
  set(PrimaryUnwind_FOUND false)
endblock()

if(BLOCK_RUN)
  set(RunCMake_TEST_FAILED)
endif()
