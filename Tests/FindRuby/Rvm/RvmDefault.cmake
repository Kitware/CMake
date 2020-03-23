set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES "")

find_package (Ruby 2.1.1 REQUIRED)
if (NOT RUBY_EXECUTABLE MATCHES "^${RUBY_HOME}/.+")
  message (FATAL_ERROR "Failed to use RVM environment: ${RUBY_EXECUTABLE}, ${RUBY_HOME}")
endif()

find_package (Ruby 2.1 REQUIRED)
if (NOT RUBY_EXECUTABLE MATCHES "^${RUBY_HOME}/.+")
  message (FATAL_ERROR "Failed to use RVM environment: ${RUBY_EXECUTABLE}, ${RUBY_HOME}")
endif()

find_package (Ruby REQUIRED)
if (NOT RUBY_EXECUTABLE MATCHES "^${RUBY_HOME}/.+")
  message (FATAL_ERROR "Failed to use RVM environment: ${RUBY_EXECUTABLE}, ${RUBY_HOME}")
endif()
