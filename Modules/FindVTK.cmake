#
# Find the native VTK includes and library
#


FIND_PATH(VTK_INCLUDE_PATH vtk.h
/usr/local/include
/usr/include
/usr/local/vtk
H:/usr/local/vtk
)

FIND_LIBRARY(VTK_LIB_PATH  vtk.dll
PATHS /usr/lib /usr/local/lib /usr/local/vtk/lib H:/usr/local/vtk/lib
)

