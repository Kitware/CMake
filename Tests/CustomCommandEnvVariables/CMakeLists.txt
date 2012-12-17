cmake_minimum_required (VERSION 2.6)
project(TestEnvVariables)

add_executable(env_test env_test.cpp)

add_custom_command(
  OUTPUT env_result.txt
  DEPENDS env_test
  COMMAND "${TestEnvVariables_BINARY_DIR}/${CMAKE_CFG_INTDIR}/env_test" arg1 arg2
  ENVIRONMENT SOME_VAR_FOR_TESTING=True ANOTHER_VAR=1
  COMMENT "custom command to test setting environment variables"
)

add_custom_target(env_result ALL DEPENDS env_result.txt)
