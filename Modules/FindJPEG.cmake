#
# Find the native JPEG includes and library
#


FIND_PATH(NATIVE_JPEG_INCLUDE_PATH jpeglib.h
/usr/local/include
/usr/include
)

FIND_LIBRARY(NATIVE_JPEG_LIB_PATH  jpeg
PATHS /usr/lib /usr/local/lib
)

