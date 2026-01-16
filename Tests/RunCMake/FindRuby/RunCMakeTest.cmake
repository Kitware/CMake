include(RunCMake)

# System Ruby tests
run_cmake(System/Ruby)
run_cmake(System/Fail)
run_cmake(System/FailExact)

# RBENV specific tests
if(CMake_TEST_FindRuby_RBENV)
  set(ENV{RBENV_ROOT} "${RBENV_ROOT}")

  # Test environment has RBENV_ROOT setup
  find_program(rbenv
    NAMES rbenv
    NAMES_PER_DIR
    PATHS "$ENV{HOME}/.rbenv/bin" ENV RBENV_ROOT
    PATH_SUFFIXES bin Scripts
    NO_CACHE)
  execute_process(COMMAND "${rbenv}" which ruby
    RESULT_VARIABLE result
    OUTPUT_VARIABLE RBENV_RUBY
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Make sure we found a valid Ruby interpreter
  if(NOT RBENV_RUBY)
    message(FATAL_ERROR "Unable to find rbenv Ruby using RBENV_ROOT=${RBENV_ROOT}")
  endif()

  # Get the version of rbenv Ruby
  execute_process(COMMAND "${RBENV_RUBY}" -e "puts RUBY_VERSION"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE RBENV_RUBY_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(result)
    message(FATAL_ERROR "Unable to detect rbenv ruby version from '${RBENV_RUBY}': ${RBENV_RUBY_VERSION}")
  endif()

  # Find system Ruby
  execute_process(COMMAND "${CMAKE_COMMAND}" -E env PATH=/usr/bin:/bin which ruby
    RESULT_VARIABLE result
    OUTPUT_VARIABLE SYSTEM_RUBY
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(SYSTEM_RUBY STREQUAL RBENV_RUBY)
    message(FATAL_ERROR "System Ruby (${SYSTEM_RUBY}) matches rbenv Ruby (${RBENV_RUBY})")
  endif()

  # Get version of the system Ruby
  execute_process(COMMAND "${SYSTEM_RUBY}" -e "puts RUBY_VERSION"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE SYSTEM_RUBY_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(result)
    message(FATAL_ERROR "Unable to detect system ruby version from '${SYSTEM_RUBY}': ${SYSTEM_RUBY_VERSION}")
  endif()

  if(SYSTEM_RUBY_VERSION VERSION_EQUAL RBENV_RUBY_VERSION)
    message(FATAL_ERROR "Your rbenv Ruby version and system Ruby version are the same (${RBENV_RUBY_VERSION}).")
  endif()

  message(STATUS "Found system Ruby (${SYSTEM_RUBY_VERSION}): ${SYSTEM_RUBY}")
  message(STATUS "Found rbenv Ruby (${RBENV_RUBY_VERSION}): ${RBENV_RUBY}")

  set(RunCMake_TEST_OPTIONS
    "-DRBENV_RUBY=${RBENV_RUBY}"
    "-DSYSTEM_RUBY=${SYSTEM_RUBY}"
    "-DRBENV_RUBY_VERSION=${RBENV_RUBY_VERSION}"
    "-DSYSTEM_RUBY_VERSION=${SYSTEM_RUBY_VERSION}")
  run_cmake(Rbenv/RbenvDefault)
  run_cmake(Rbenv/RbenvOnly)
  run_cmake(Rbenv/RbenvStandard)
  unset(RunCMake_TEST_OPTIONS)
endif()

# RVM specific tests
if(CMake_TEST_FindRuby_RVM)
  # Properly using rvm would require sourcing a shell script, eg `source "$HOME/.rvm/scripts/rvm"`
  # Instead, we just rely on the env variable MY_RUBY_HOME
  if(NOT MY_RUBY_HOME)
    message(FATAL_ERROR "MY_RUBY_HOME should be set to a valid RVM ruby location, or you should call `rvm use x.y.z` before")
  endif()

  set(ENV{MY_RUBY_HOME} "${MY_RUBY_HOME}")

  execute_process(COMMAND "${MY_RUBY_HOME}/bin/ruby" -e "puts RUBY_VERSION"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE RVM_RUBY_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(result)
    message(FATAL_ERROR "Unable to detect RVM ruby version from '${MY_RUBY_HOME}/bin/ruby': ${RVM_RUBY_VERSION}")
  endif()

  execute_process(COMMAND "${CMAKE_COMMAND}" -E env --unset=MY_RUBY_HOME PATH=/usr/bin:/bin which ruby
    RESULT_VARIABLE result
    OUTPUT_VARIABLE SYSTEM_RUBY
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(SYSTEM_RUBY MATCHES "^${MY_RUBY_HOME}")
    message(FATAL_ERROR "Unable to find system ruby, found ${SYSTEM_RUBY} which is part of MY_RUBY_HOME=${MY_RUBY_HOME}")
  endif()

  # Check version of the system ruby executable.
  execute_process(COMMAND "${SYSTEM_RUBY}" -e "puts RUBY_VERSION"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE SYSTEM_RUBY_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(result)
    message(FATAL_ERROR "Unable to detect system ruby version from '${SYSTEM_RUBY}': ${SYSTEM_RUBY_VERSION}")
  endif()

  if(SYSTEM_RUBY_VERSION VERSION_EQUAL RVM_RUBY_VERSION)
    message(FATAL_ERROR "Your RVM Ruby version and system Ruby version are the same (${RVM_RUBY_VERSION}).")
  endif()

  message(STATUS "Found system Ruby (${SYSTEM_RUBY_VERSION}): ${SYSTEM_RUBY}")
  message(STATUS "Found RVM Ruby (${RVM_RUBY_VERSION}): ${MY_RUBY_HOME}/bin/ruby")

  set(ORIGINAL_PATH "$ENV{PATH}")

  # RvmDefault - just MY_RUBY_HOME set
  set(RunCMake_TEST_OPTIONS "-DMY_RUBY_HOME=${MY_RUBY_HOME}")
  run_cmake(Rvm/RvmDefault)

  # RvmOnly - PATH unset, MY_RUBY_HOME set
  unset(ENV{PATH})
  set(RunCMake_TEST_OPTIONS
    "-DMY_RUBY_HOME=${MY_RUBY_HOME}"
    "-DRVM_RUBY_VERSION=${RVM_RUBY_VERSION}"
    "-DSYSTEM_RUBY_VERSION=${SYSTEM_RUBY_VERSION}")
  run_cmake(Rvm/RvmOnly)

  # UnsetRvmOnly - MY_RUBY_HOME unset, PATH set to minimal
  unset(ENV{MY_RUBY_HOME})
  set(ENV{PATH} "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin")
  set(RunCMake_TEST_OPTIONS
    "-DRVM_RUBY_VERSION=${RVM_RUBY_VERSION}"
    "-DSYSTEM_RUBY_VERSION=${SYSTEM_RUBY_VERSION}")
  run_cmake(Rvm/UnsetRvmOnly)

  # RvmStandard - PATH minimal, MY_RUBY_HOME set
  set(ENV{MY_RUBY_HOME} "${MY_RUBY_HOME}")
  set(RunCMake_TEST_OPTIONS "-DMY_RUBY_HOME=${MY_RUBY_HOME}")
  run_cmake(Rvm/RvmStandard)

  # Restore PATH
  set(ENV{PATH} "${ORIGINAL_PATH}")
  unset(RunCMake_TEST_OPTIONS)
endif()
