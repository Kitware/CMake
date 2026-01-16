set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES "")

set(Ruby_FIND_VIRTUALENV ONLY)
unset(ENV{MY_RUBY_HOME}) # Suppress RVM code path for this test.

# We should find the rbenv Ruby not standard Ruby
unset(Ruby_EXECUTABLE CACHE)
unset(RUBY_EXECUTABLE) # compatibility variable
find_package (Ruby QUIET)
if (NOT Ruby_EXECUTABLE STREQUAL "${RBENV_RUBY}")
  message (FATAL_ERROR "Failed to find rbenv Ruby: ${Ruby_EXECUTABLE}")
endif()

# We should find the rbenv Ruby not standard Ruby
unset(Ruby_EXECUTABLE CACHE)
unset(RUBY_EXECUTABLE) # compatibility variable
find_package (Ruby ${RBENV_RUBY_VERSION} QUIET)
if (NOT Ruby_EXECUTABLE STREQUAL "${RBENV_RUBY}")
  message (FATAL_ERROR "Failed to find rbenv Ruby: ${Ruby_EXECUTABLE}")
endif()

# We should not find standard Ruby
unset(Ruby_EXECUTABLE CACHE)
unset(RUBY_EXECUTABLE) # compatibility variable
find_package (Ruby ${SYSTEM_RUBY_VERSION} EXACT QUIET)
if (NOT Ruby_EXECUTABLE STREQUAL "Ruby_EXECUTABLE-NOTFOUND")
  message (FATAL_ERROR "Incorrectly found Ruby: ${Ruby_EXECUTABLE}")
endif()
