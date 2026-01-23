set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES "")

set(Ruby_FIND_VIRTUALENV ONLY)
set(Ruby_RBENV_EXECUTABLE "") # Suppress rbenv code path for this test.

# If ENV{MY_RUBY_HOME} isn't defined and Ruby_FIND_VIRTUALENV is set to ONLY
# then Ruby should not be found
find_package (Ruby ${RVM_RUBY_VERSION} EXACT QUIET)
if (Ruby_FOUND)
  message (FATAL_ERROR "Incorrectly found Ruby: ${Ruby_EXECUTABLE}")
endif()

# it should *not* find the system ruby
find_package (Ruby ${SYSTEM_RUBY_VERSION} EXACT QUIET)
if (Ruby_FOUND)
  message (FATAL_ERROR "Incorrectly found Ruby: ${Ruby_EXECUTABLE}")
endif()
