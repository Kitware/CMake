cmake_minimum_required(VERSION 3.10)
project(TestFindASPELL C)
include(CTest)

find_package(ASPELL COMPONENTS ASPELL)

add_executable(test_tgt main.c)
target_link_libraries(test_tgt ASPELL::ASPELL)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${ASPELL_INCLUDE_DIR})
target_link_libraries(test_var PRIVATE ${ASPELL_LIBRARIES})
add_test(NAME test_var COMMAND test_var)

set_tests_properties(
  test_tgt test_var
  PROPERTIES PASS_REGULAR_EXPRESSION "^Word \"[^\"]+\" is spelled correctly"
)

find_package(ASPELL)
add_executable(test_version version.c)
target_link_libraries(test_version ASPELL::ASPELL)
target_compile_definitions(
  test_version PRIVATE -DCMAKE_EXPECTED_ASPELL_VERSION="${ASPELL_VERSION}"
)
add_test(NAME test_version COMMAND test_version)
