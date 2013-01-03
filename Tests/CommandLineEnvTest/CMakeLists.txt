cmake_minimum_required (VERSION 2.6)
project(CommandLineEnvTest)

# Set TEST_ENV_A and TEST_ENV_B and execute cmake -E environment, which echos
# all environment variables
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env TEST_ENV_A=1 TEST_ENV_B=2 ${CMAKE_COMMAND} -E environment
  OUTPUT_VARIABLE ENV_TEST_OUTPUT
  RESULT_VARIABLE ENV_TEST_RESULT)

# Look for the environment veriables in the outputs
string(REGEX MATCH "TEST_ENV_A=1" TEST_ENV_A_FOUND "${ENV_TEST_OUTPUT}")
string(REGEX MATCH "TEST_ENV_B=2" TEST_ENV_B_FOUND "${ENV_TEST_OUTPUT}")

# Complain if they're not found
if(NOT TEST_ENV_A_FOUND)
  message(SEND_ERROR "TEST_ENV_A environment variable not found")
endif()

if(NOT TEST_ENV_B_FOUND)
  message(SEND_ERROR "TEST_ENV_B environment variable not found")
endif()

add_executable(CommandLineEnvTest CommandLineEnvTest.cxx)
