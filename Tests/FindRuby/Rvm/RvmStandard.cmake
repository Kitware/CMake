set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES "")

set (Ruby_FIND_VIRTUALENV STANDARD)
find_package (Ruby REQUIRED)

if (RUBY_EXECUTABLE MATCHES "^${RUBY_HOME}/.+")
  message (FATAL_ERROR "RVM ruby unexpectedly found at ${RUBY_EXECUTABLE}, matches ${RUBY_HOME}")
endif()
