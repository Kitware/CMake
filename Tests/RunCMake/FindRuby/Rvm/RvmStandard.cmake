set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES "")

set (Ruby_FIND_VIRTUALENV STANDARD)
find_package (Ruby REQUIRED)

if (Ruby_EXECUTABLE MATCHES "^${MY_RUBY_HOME}")
  message (FATAL_ERROR "RVM ruby unexpectedly found at ${Ruby_EXECUTABLE}, matches ${MY_RUBY_HOME}")
endif()
