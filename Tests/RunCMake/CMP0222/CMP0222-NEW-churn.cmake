cmake_policy(SET CMP0222 NEW)

# The compatibility break the policy covers: under NEW the keyword is
# consumed as an operator, so a variable of that name is a hard error.
set(PATH_IS_PREFIX "yes")
if(NOT PATH_IS_PREFIX STREQUAL "yes")
  message(SEND_ERROR "unreachable")
endif()
