include(CTest)
add_subdirectory(SubDir)
add_custom_target(TopFail ALL COMMAND does_not_exist)
add_test(NAME TopTest COMMAND ${CMAKE_COMMAND} -E echo "Running TopTest")
install(CODE [[
  message(FATAL_ERROR "Installing Top")
]])
