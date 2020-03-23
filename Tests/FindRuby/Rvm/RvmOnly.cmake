set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES "")

set(Ruby_FIND_VIRTUALENV ONLY)

# Test: FindRuby.RvmOnly
if (RUBY_HOME)
  # => Trying to find exactly system ruby using ONLY virtual environment should fail
  find_package (Ruby ${SYSTEM_RUBY_VERSION} EXACT QUIET)
  if(Ruby_FOUND)
    message (FATAL_ERROR "Ruby unexpectedly found.")
  endif()
  # And should work to find the rvm version
  find_package (Ruby ${RVM_RUBY_VERSION} EXACT QUIET)
  if(Ruby_FOUND)
    message (FATAL_ERROR "Ruby unexpectedly found.")
  endif()
endif()


# Test: FindRuby.UnsetRvmOnly
if (NOT RUBY_HOME)

  # If ENV{MY_RUBY_HOME} isn't defined, it should default back to "STANDARD"
  # At which point:

  # It shouldn't find the RVM ruby
  find_package (Ruby ${RVM_RUBY_VERSION} EXACT QUIET)
  if(Ruby_FOUND)
    message(FATAL_ERROR "Found RVM ruby when expecting system")
  endif()

  # it should find the system ruby
  find_package (Ruby ${SYSTEM_RUBY_VERSION} EXACT QUIET)
  if(NOT Ruby_FOUND)
    message (FATAL_ERROR "Ruby not found.")
  endif()
  if (Ruby_FOUND MATCHES "^${RUBY_HOME}/.+")
    message(FATAL_ERROR "Failed to find system ruby")
  endif()
endif()
