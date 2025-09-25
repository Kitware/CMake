set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES "")

set (Ruby_FIND_VIRTUALENV STANDARD)
unset(ENV{MY_RUBY_HOME}) # Suppress RVM code path for this test.

# We should find the standard Ruby
unset(Ruby_EXECUTABLE CACHE)
unset(RUBY_EXECUTABLE) # compatibility variable
find_package (Ruby QUIET)
if (NOT Ruby_EXECUTABLE STREQUAL "${SYSTEM_RUBY}")
  message (FATAL_ERROR "Failed to find standard Ruby: ${Ruby_EXECUTABLE}")
endif()

# We should not find the rbenv Ruby or standard Ruby
unset(Ruby_EXECUTABLE CACHE)
unset(RUBY_EXECUTABLE) # compatibility variable
find_package (Ruby ${RBENV_RUBY_VERSION} EXACT QUIET)
if (NOT Ruby_EXECUTABLE STREQUAL "Ruby_EXECUTABLE-NOTFOUND")
  message (FATAL_ERROR "Incorrectly found rbenv Ruby: ${Ruby_EXECUTABLE}")
endif()

# We should find standard Ruby
unset(Ruby_EXECUTABLE CACHE)
unset(RUBY_EXECUTABLE) # compatibility variable
find_package (Ruby ${SYSTEM_RUBY_VERSION} EXACT QUIET)
if (NOT Ruby_EXECUTABLE MATCHES "${SYSTEM_RUBY}")
  message (FATAL_ERROR "Failed to find standard Ruby: ${Ruby_EXECUTABLE}")
endif()
