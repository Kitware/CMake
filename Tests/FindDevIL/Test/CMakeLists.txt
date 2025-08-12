cmake_minimum_required(VERSION 3.10)
project(TestFindDevIL C)
include(CTest)

find_package(DevIL)

add_executable(test_devil_var main.c)
target_include_directories(test_devil_var PRIVATE ${IL_INCLUDE_DIRS})
target_link_libraries(test_devil_var PRIVATE ${IL_LIBRARIES})
target_compile_definitions(
  test_devil_var
  PRIVATE CMAKE_EXPECTED_DEVIL_VERSION="${DevIL_VERSION}"
)
add_test(NAME test_devil_var COMMAND test_devil_var)

add_executable(test_devil_il_tgt main.c)
target_link_libraries(test_devil_il_tgt DevIL::IL)
target_compile_definitions(
  test_devil_il_tgt
  PRIVATE CMAKE_EXPECTED_DEVIL_VERSION="${DevIL_VERSION}"
)
add_test(NAME test_devil_il_tgt COMMAND test_devil_il_tgt)

add_executable(test_devil_ilu_tgt main_ilu.c)
target_link_libraries(test_devil_ilu_tgt DevIL::ILU)

add_executable(test_devil_ilu_var main_ilu.c)
target_include_directories(test_devil_ilu_var PRIVATE ${IL_INCLUDE_DIRS} ${ILU_INCLUDE_DIRS})
target_link_libraries(test_devil_ilu_var ${IL_LIBRARIES} ${ILU_LIBRARIES})

#FIXME: Check DevIL::ILUT target
