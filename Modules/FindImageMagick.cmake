#
# This module finds if ImageMagick tools are installed and determines 
# where the executables are. This code sets the following variables:
#
#  IMAGEMAGICK_CONVERT_EXECUTABLE   = the full path to the 'convert' utility 
#  IMAGEMAGICK_MOGRIFY_EXECUTABLE   = the full path to the 'mogrify' utility 
#  IMAGEMAGICK_IMPORT_EXECUTABLE    = the full path to the 'import'  utility 
#  IMAGEMAGICK_MONTAGE_EXECUTABLE   = the full path to the 'montage' utility 
#  IMAGEMAGICK_COMPOSITE_EXECUTABLE = the full path to the 'composite' utility 
#

FIND_PROGRAM(IMAGEMAGICK_CONVERT_EXECUTABLE
  NAMES convert
)

FIND_PROGRAM(IMAGEMAGICK_MOGRIFY_EXECUTABLE
  NAMES mogrify
)

FIND_PROGRAM(IMAGEMAGICK_IMPORT_EXECUTABLE
  NAMES import
)

FIND_PROGRAM(IMAGEMAGICK_MONTAGE_EXECUTABLE
  NAMES montage
)

FIND_PROGRAM(IMAGEMAGICK_COMPOSITE_EXECUTABLE
  NAMES composite
)




