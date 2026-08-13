cmake_minimum_required(VERSION 4.0)

# CMP0157 must be set before the Swift language is enabled.
cmake_policy(SET CMP0157 OLD)
cmake_policy(SET CMP0195 OLD)
cmake_policy(SET CMP0215 NEW)

if(NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(SEND_ERROR "this test must use a Ninja generator, found ${CMAKE_GENERATOR}")
endif()

# CMP0215=NEW requires both CMP0157=NEW and CMP0195=NEW; neither is set here,
# so enabling the Swift language must fail and name both policies.
enable_language(Swift)
