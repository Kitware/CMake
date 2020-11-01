cmake_minimum_required(VERSION 3.17)
project(TestRVM LANGUAGES NONE)

include(CTest)

# To run this test, you need to have at least one RVM ruby installed
# and to ensure that the env variable 'MY_RUBY_HOME' is set to a valid RVM ruby when you run the test
# (which is the case if you have done `rvm use x.y.z`, but could be manually set too)

# Properly using rvm would require sourcing a shell script, eg `source "$HOME/.rvm/scripts/rvm"`
# Instead, I'll just rely on the env variable MY_RUBY_HOME
set(MY_RUBY_HOME "$ENV{MY_RUBY_HOME}")
if(NOT MY_RUBY_HOME)
  message(FATAL_ERROR "Env variable MY_RUBY_HOME should be set to a valid RVM ruby location, or you should call `rvm use x.y.z` before")
endif()
execute_process (COMMAND "${MY_RUBY_HOME}/bin/ruby" -e "puts RUBY_VERSION"
                 RESULT_VARIABLE result
                 OUTPUT_VARIABLE RVM_RUBY_VERSION
                 ERROR_QUIET
                 OUTPUT_STRIP_TRAILING_WHITESPACE)

if (result)
  message (FATAL_ERROR "Unable to detect RVM ruby version from `${MY_RUBY_HOME}/bin/ruby`: ${RVM_RUBY_VERSION}")
endif()

execute_process(COMMAND "${CMAKE_COMMAND}" -E env --unset=MY_RUBY_HOME PATH=/usr/bin:/bin
                        "which" "ruby"
                 RESULT_VARIABLE result
                 OUTPUT_VARIABLE SYSTEM_RUBY
                 ERROR_QUIET
                 OUTPUT_STRIP_TRAILING_WHITESPACE)

if (SYSTEM_RUBY MATCHES "^${MY_RUBY_HOME}/.+")
  message(FATAL_ERROR "Unable to find system ruby, found ${SYSTEM_RUBY} which is part of MY_RUBY_HOME=${MY_RUBY_HOME}")
endif()

# Check version of the system ruby executable.
execute_process (COMMAND "${SYSTEM_RUBY}" -e "puts RUBY_VERSION"
                 RESULT_VARIABLE result
                 OUTPUT_VARIABLE SYSTEM_RUBY_VERSION
                 ERROR_QUIET
                 OUTPUT_STRIP_TRAILING_WHITESPACE)

if (result)
  message (FATAL_ERROR "Unable to detect system ruby version from '${SYSTEM_RUBY}': ${SYSTEM_RUBY_VERSION}")
endif()

if(SYSTEM_RUBY_VERSION VERSION_EQUAL RVM_RUBY_VERSION)
  message(FATAL_ERROR "Your RVM Ruby Version and your System ruby version are the same (${RVM_RUBY_VERSION}).")
endif()

message("Found System Ruby (${SYSTEM_RUBY_VERSION}): ${SYSTEM_RUBY}")
message("Found  RVM   Ruby (${RVM_RUBY_VERSION}): ${MY_RUBY_HOME}/bin/ruby")

add_test(NAME FindRuby.RvmDefault
         COMMAND "${CMAKE_COMMAND}" -E env "MY_RUBY_HOME=${MY_RUBY_HOME}"
                 "${CMAKE_COMMAND}" "-DRUBY_HOME=${MY_RUBY_HOME}"
                 -P "${CMAKE_CURRENT_LIST_DIR}/RvmDefault.cmake")

add_test(NAME FindRuby.RvmOnly
  COMMAND "${CMAKE_COMMAND}" -E env --unset=PATH
                                    "MY_RUBY_HOME=${MY_RUBY_HOME}"
          "${CMAKE_COMMAND}" "-DRUBY_HOME=${MY_RUBY_HOME}"
                 "-DRVM_RUBY_VERSION=${RVM_RUBY_VERSION}" "-DSYSTEM_RUBY_VERSION=${SYSTEM_RUBY_VERSION}"
                 -P "${CMAKE_CURRENT_LIST_DIR}/RvmOnly.cmake")
add_test(NAME FindRuby.UnsetRvmOnly
         COMMAND "${CMAKE_COMMAND}" -E env --unset=MY_RUBY_HOME "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin"
                 "${CMAKE_COMMAND}" "-DRVM_RUBY_VERSION=${RVM_RUBY_VERSION}" "-DSYSTEM_RUBY_VERSION=${SYSTEM_RUBY_VERSION}"
                 -P "${CMAKE_CURRENT_LIST_DIR}/RvmOnly.cmake")

add_test(NAME FindRuby.RvmStandard
         COMMAND "${CMAKE_COMMAND}" -E env "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin"
                                           "MY_RUBY_HOME=${MY_RUBY_HOME}"
                 "${CMAKE_COMMAND}" "-DRUBY_HOME=${MY_RUBY_HOME}"
                 -P "${CMAKE_CURRENT_LIST_DIR}/RvmStandard.cmake")
