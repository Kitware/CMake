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
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check.cmake
  COMMAND ${CMAKE_COMMAND} -E echo "check done"
  VERBATIM
  )
