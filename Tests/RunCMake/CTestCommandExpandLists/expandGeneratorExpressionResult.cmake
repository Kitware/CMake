include(CTest)


set(cmp_args "1ARG=COMMAND_EXPAND_LISTS" "2ARG=test" "3ARG=outfile"
  "4ARG=content")
set(AARGS "")
foreach(arg IN LISTS cmp_args)
  list(APPEND AARGS "-DA${arg}")
endforeach()



add_test(
  NAME CommandExpandList
  COMMAND ${CMAKE_COMMAND} ${AARGS} -V
  "-DB$<JOIN:${cmp_args},;-DB>"
  "-P" "${CMAKE_CURRENT_LIST_DIR}/compare_options.cmake"
  COMMAND_EXPAND_LISTS
)
