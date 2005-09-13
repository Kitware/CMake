# a simple test cas
project (OutOfSource)

add_subdirectory(SubDir)

get_directory_property(ANIMAL DIRECTORY OutOfSourceSubdir DEFINITION WEASELS)

configure_file(
  ${OutOfSource_SOURCE_DIR}/testdp.h.in
  ${OutOfSource_BINARY_DIR}/SubDir/OutOfSourceSubdir/testdp.h
  )

set(KEN 1)
