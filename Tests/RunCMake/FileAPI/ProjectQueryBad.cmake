# All of these should fail. Execution does continue though, so we should see
# the error output from each one. There is no observable effect of the command
# during the configure phase, so it isn't critical to end processing on the
# first failure. Allowing execution to proceed may allow the project to see
# other potential errors before ultimately halting. That behavior is generally
# desirable, and the multiple failing calls here will confirm that we retain
# that behavior.

message(NOTICE "Non-query check")
cmake_file_api(NOT_A_QUERY)

message(NOTICE "Invalid API version checks")
cmake_file_api(QUERY API_VERSION 2)
cmake_file_api(QUERY API_VERSION nah)

message(NOTICE "Invalid version numbers check")
cmake_file_api(
  QUERY
  API_VERSION 1
  CODEMODEL nope
  CACHE -2
  CMAKEFILES .8
  TOOLCHAINS 2 0.1
)

message(NOTICE "Requested versions too high check")
cmake_file_api(
  QUERY
  API_VERSION 1
  CODEMODEL 3
  CACHE 3
  CMAKEFILES 2
  TOOLCHAINS 1.1
)

message(NOTICE "Requested versions too low check")
cmake_file_api(
  QUERY
  API_VERSION 1
  CODEMODEL 1
  CACHE 1
)
