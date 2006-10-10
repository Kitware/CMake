PROJECT(ANOTHER_PROJ)

# subdir to an out of source and out of binary directory
ADD_SUBDIRECTORY(${OutOfSource_SOURCE_DIR}/../OutOfBinary
                 ${OutOfSource_BINARY_DIR}/../OutOfBinary)

# subdir to a sibling dir
ADD_SUBDIRECTORY(${OutOfSource_SOURCE_DIR}/${KEN}OutOfSourceSubdir OutOfSourceSubdir )
