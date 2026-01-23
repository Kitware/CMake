set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES "")

set(Ruby_FIND_VIRTUALENV ONLY)
set(Ruby_RBENV_EXECUTABLE "") # Suppress rbenv code path for this test.

# Trying to find the exact system ruby version using ONLY virtual environment should fail
find_package (Ruby ${SYSTEM_RUBY_VERSION} EXACT QUIET)
if (Ruby_FOUND)
  message (FATAL_ERROR "Incorrectly found system Ruby: ${Ruby_EXECUTABLE}, ${MY_RUBY_HOME}")
endif()

# Finding RVM version should work
find_package (Ruby ${RVM_RUBY_VERSION} EXACT QUIET)
if (!Ruby_FOUND)
  message (FATAL_ERROR "Failed to find RVM Ruby: ${MY_RUBY_HOME}")
endif()
