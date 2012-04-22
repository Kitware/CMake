string(REGEX REPLACE "^/(.*)" "\\1" REL_CMAKE_DATA_DIR "${CMAKE_DATA_DIR}")
ADD_SUBDIRECTORY (bash-completion)
