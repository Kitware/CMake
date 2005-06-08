PROJECT(ANOTHER_PROJ)
# subdir to a sibling dir
ADD_SUBDIRECTORY(OutOfSourceSubdir 
  ${OutOfSource_SOURCE_DIR}/${KEN}OutOfSourceSubdir)
