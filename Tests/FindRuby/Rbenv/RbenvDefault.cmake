set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES "")

unset(ENV{MY_RUBY_HOME}) # Suppress RVM code path for this test.

# Should find rbenv Ruby
unset(Ruby_EXECUTABLE CACHE)
unset(RUBY_EXECUTABLE) # compatibility variable
find_package (Ruby REQUIRED)
if (NOT "${Ruby_EXECUTABLE}" STREQUAL "${RBENV_RUBY}")
  message (FATAL_ERROR "Failed to find rbenv Ruby: ${Ruby_EXECUTABLE}")
endif()

# Should find rbenv Ruby
unset(Ruby_EXECUTABLE CACHE)
unset(RUBY_EXECUTABLE) # compatibility variable
find_package (Ruby ${RBENV_RUBY_VERSION} REQUIRED)
if (NOT Ruby_EXECUTABLE MATCHES "${RBENV_RUBY}")
  message (FATAL_ERROR "Failed to find rbenv Ruby: ${Ruby_EXECUTABLE}")
endif()

# Should find standard Ruby
unset(Ruby_EXECUTABLE CACHE)
unset(RUBY_EXECUTABLE) # compatibility variable
find_package (Ruby ${SYSTEM_RUBY_VERSION} EXACT)
if (NOT Ruby_EXECUTABLE STREQUAL "${SYSTEM_RUBY}")
  message (FATAL_ERROR "Failed to find system Ruby: ${Ruby_EXECUTABLE}")
endif()
