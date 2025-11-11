include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/LowerCase-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

function(expect_in_list list value)
  list(FIND ${list} "${value}" index)
  if(${index} EQUAL -1)
    set(RunCMake_TEST_FAILED
      "Expected '${value}' in ${list} ('${${list}}'), but it was not found" PARENT_SCOPE)
  endif()
endfunction()

file(GLOB files
  LIST_DIRECTORIES false
  RELATIVE "${out_dir}"
  "${out_dir}/*.cps"
)
expect_in_list(files "farm.cps")
expect_in_list(files "farm-extra.cps")

file(READ "${out_dir}/farm.cps" content)
expect_value("${content}" "Farm" "name")
expect_value("${content}" "interface" "components" "Cow" "type")

file(READ "${out_dir}/farm-extra.cps" content)
expect_value("${content}" "Farm" "name")
expect_value("${content}" "interface" "components" "Pig" "type")
