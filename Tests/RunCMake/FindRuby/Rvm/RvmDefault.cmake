set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES "")

find_package (Ruby 2.1.1 REQUIRED)
if (NOT Ruby_EXECUTABLE MATCHES "^${MY_RUBY_HOME}")
  message (FATAL_ERROR "Failed to find RVM Ruby: ${Ruby_EXECUTABLE}, ${MY_RUBY_HOME}")
endif()

find_package (Ruby 2.1 REQUIRED)
if (NOT Ruby_EXECUTABLE MATCHES "^${MY_RUBY_HOME}")
  message (FATAL_ERROR "Failed to find RVM Ruby: ${Ruby_EXECUTABLE}, ${MY_RUBY_HOME}")
endif()

find_package (Ruby REQUIRED)
if (NOT Ruby_EXECUTABLE MATCHES "^${MY_RUBY_HOME}")
  message (FATAL_ERROR "Failed to find RVM Ruby: ${Ruby_EXECUTABLE}, ${MY_RUBY_HOME}")
endif()
