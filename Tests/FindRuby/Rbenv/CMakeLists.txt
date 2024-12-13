cmake_minimum_required(VERSION 3.31)
project(TestRBENV LANGUAGES NONE)

include(CTest)

# Test environment has RBENV_ROOT setup
find_program (rbenv
        NAMES rbenv
        NAMES_PER_DIR
        PATHS "$ENV{HOME}/.rbenv/bin/rbenv" ENV RBENV_ROOT
        PATH_SUFFIXES bin Scripts
        NO_CACHE)
execute_process (COMMAND "${rbenv}" "which" "ruby"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE RBENV_RUBY
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)

# Make sure we found a valid Ruby interpreter
if(NOT RBENV_RUBY)
  message(FATAL_ERROR "Unable to find RBENV using RBENV_ROOT ${RBENV_ROOT}")
endif()

# Get the version of rbenv Ruby
execute_process (COMMAND "${RBENV_RUBY}" -e "puts RUBY_VERSION"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE RBENV_RUBY_VERSION
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE)

if (result)
  message (FATAL_ERROR "Unable to detect rbenv ruby version from '${RBENV_RUBY}': ${RBENV_RUBY_VERSION}")
endif()

# Find system Ruby
execute_process(COMMAND "${CMAKE_COMMAND}" -E env PATH=/usr/bin:/bin "which" "ruby"
                RESULT_VARIABLE result
                OUTPUT_VARIABLE SYSTEM_RUBY
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if (SYSTEM_RUBY MATCHES "${RBENV_RUBY}")
  message(FATAL_ERROR "System Ruby (${SYSTEM_RUBY}) matches rbenv Ruby (${RBENV_RUBY})")
endif()

# Get version of the system Ruby
execute_process (COMMAND "${SYSTEM_RUBY}" -e "puts RUBY_VERSION"
                 RESULT_VARIABLE result
                 OUTPUT_VARIABLE SYSTEM_RUBY_VERSION
                 ERROR_QUIET
                 OUTPUT_STRIP_TRAILING_WHITESPACE)

if (result)
  message (FATAL_ERROR "Unable to detect system ruby version from '${SYSTEM_RUBY}': ${SYSTEM_RUBY_VERSION}")
endif()

if(SYSTEM_RUBY_VERSION VERSION_EQUAL RBENV_RUBY_VERSION)
  message(FATAL_ERROR "Your RBENV Ruby Version and standard ruby version are the same (${RBENV_RUBY_VERSION}).")
endif()

message(STATUS "Found standard Ruby (${SYSTEM_RUBY_VERSION}): ${SYSTEM_RUBY}")
message(STATUS "Found rbenv Ruby (${RBENV_RUBY_VERSION}): ${RBENV_RUBY}")

add_test(NAME FindRuby.RbenvDefault
        COMMAND "${CMAKE_COMMAND}" -E env "RBENV_ROOT=$ENV{RBENV_ROOT}"
                "${CMAKE_COMMAND}"  "-DRBENV_RUBY=${RBENV_RUBY}" "-DSYSTEM_RUBY=${SYSTEM_RUBY}" "-DRBENV_RUBY_VERSION=${RBENV_RUBY_VERSION}" "-DSYSTEM_RUBY_VERSION=${SYSTEM_RUBY_VERSION}"
                -P "${CMAKE_CURRENT_LIST_DIR}/RbenvDefault.cmake")

add_test(NAME FindRuby.RbenvOnly
         COMMAND "${CMAKE_COMMAND}" -E env "RBENV_ROOT=$ENV{RBENV_ROOT}"
                 "${CMAKE_COMMAND}"  "-DRBENV_RUBY=${RBENV_RUBY}" "-DSYSTEM_RUBY=${SYSTEM_RUBY}" "-DRBENV_RUBY_VERSION=${RBENV_RUBY_VERSION}" "-DSYSTEM_RUBY_VERSION=${SYSTEM_RUBY_VERSION}"
                 -P "${CMAKE_CURRENT_LIST_DIR}/RbenvOnly.cmake")

add_test(NAME FindRuby.RbenvStandard
        COMMAND "${CMAKE_COMMAND}" -E env "RBENV_ROOT=$ENV{RBENV_ROOT}"
                "${CMAKE_COMMAND}"  "-DRBENV_RUBY=${RBENV_RUBY}" "-DSYSTEM_RUBY=${SYSTEM_RUBY}" "-DRBENV_RUBY_VERSION=${RBENV_RUBY_VERSION}" "-DSYSTEM_RUBY_VERSION=${SYSTEM_RUBY_VERSION}"
                -P "${CMAKE_CURRENT_LIST_DIR}/RbenvStandard.cmake")
