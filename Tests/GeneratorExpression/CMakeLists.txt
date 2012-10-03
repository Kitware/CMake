cmake_minimum_required (VERSION 2.8.8)
project(GeneratorExpression NONE)

add_custom_target(check ALL
  COMMAND ${CMAKE_COMMAND}
    -Dtest_0=$<0:nothing>
    -Dtest_1=$<1:content>
    -Dconfig=$<CONFIGURATION>
    -Dtest_and_0=$<AND:0>
    -Dtest_and_0_0=$<AND:0,0>
    -Dtest_and_0_1=$<AND:0,1>
    -Dtest_and_1=$<AND:1>
    -Dtest_and_1_0=$<AND:1,0>
    -Dtest_and_1_1=$<AND:1,1>
    -Dtest_config_0=$<CONFIG:$<CONFIGURATION>x>
    -Dtest_config_1=$<CONFIG:$<CONFIGURATION>>
    -Dtest_not_0=$<NOT:0>
    -Dtest_not_1=$<NOT:1>
    -Dtest_or_0=$<OR:0>
    -Dtest_or_0_0=$<OR:0,0>
    -Dtest_or_0_1=$<OR:0,1>
    -Dtest_or_1=$<OR:1>
    -Dtest_or_1_0=$<OR:1,0>
    -Dtest_or_1_1=$<OR:1,1>
    -Dtest_bool_notfound=$<BOOL:NOTFOUND>
    -Dtest_bool_foo_notfound=$<BOOL:Foo-NOTFOUND>
    -Dtest_bool_true=$<BOOL:True>
    -Dtest_bool_false=$<BOOL:False>
    -Dtest_bool_on=$<BOOL:On>
    -Dtest_bool_off=$<BOOL:Off>
    -Dtest_bool_no=$<BOOL:No>
    -Dtest_bool_n=$<BOOL:N>
    -Dtest_strequal_yes_yes=$<STREQUAL:Yes,Yes>
    -Dtest_strequal_yes_yes_cs=$<STREQUAL:Yes,yes>
    -Dtest_strequal_yes_no=$<STREQUAL:Yes,No>
    -Dtest_strequal_no_yes=$<STREQUAL:No,Yes>
    -Dtest_strequal_angle_r=$<STREQUAL:$<ANGLE-R>,$<ANGLE-R>>
    -Dtest_strequal_comma=$<STREQUAL:$<COMMA>,$<COMMA>>
    -Dtest_strequal_angle_r_comma=$<STREQUAL:$<ANGLE-R>,$<COMMA>>
    -Dtest_strequal_both_empty=$<STREQUAL:,>
    -Dtest_strequal_one_empty=$<STREQUAL:something,>
    -Dtest_angle_r=$<ANGLE-R>
    -Dtest_comma=$<COMMA>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check.cmake
  COMMAND ${CMAKE_COMMAND} -E echo "check done"
  VERBATIM
  )
