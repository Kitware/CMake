# - Find Image Magick
# This module finds if ImageMagick tools are installed and determines 
# where the executables are. This code sets the following variables:
#
#  IMAGEMAGICK_CONVERT_EXECUTABLE   = 
#     the full path to the 'convert' utility 
#  IMAGEMAGICK_MOGRIFY_EXECUTABLE   = 
#     the full path to the 'mogrify' utility 
#  IMAGEMAGICK_IMPORT_EXECUTABLE    = 
#     the full path to the 'import'  utility 
#  IMAGEMAGICK_MONTAGE_EXECUTABLE   = 
#     the full path to the 'montage' utility 
#  IMAGEMAGICK_COMPOSITE_EXECUTABLE = 
#     the full path to the 'composite' utility 
#

IF (WIN32)

  # Try to find the ImageMagick binary path.

  FIND_PATH(IMAGEMAGICK_BINARY_PATH mogrify.exe
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]
    DOC "Path to the ImageMagick binary directory where all executable should be found."
  )

  # Be extra-careful here: we do NOT want CMake to look in the system's PATH
  # env var to search for convert.exe, otherwise it is going to pick
  # Window's own convert.exe, and you may say good-bye to your disk.

  FIND_PROGRAM(IMAGEMAGICK_CONVERT_EXECUTABLE
    NAMES convert
    PATHS ${IMAGEMAGICK_BINARY_PATH}
    NO_SYSTEM_PATH
    DOC "Path to ImageMagick's convert executable. WARNING: note that this tool, named convert.exe, conflicts with Microsoft Window's own convert.exe, which is used to convert FAT partitions to NTFS format ! Therefore, be extra-careful and make sure the right convert.exe has been picked."
  )

ELSE (WIN32)

  SET (IMAGEMAGICK_BINARY_PATH "")

  FIND_PROGRAM(IMAGEMAGICK_CONVERT_EXECUTABLE
    NAMES convert
    PATHS ${IMAGEMAGICK_BINARY_PATH}
    DOC "Path to ImageMagick's convert executable."
  )

ENDIF (WIN32)

# Find mogrify, import, montage, composite

FIND_PROGRAM(IMAGEMAGICK_MOGRIFY_EXECUTABLE
  NAMES mogrify
  PATHS ${IMAGEMAGICK_BINARY_PATH}
  DOC "Path to ImageMagick's mogrify executable."
)

FIND_PROGRAM(IMAGEMAGICK_IMPORT_EXECUTABLE
  NAMES import
  PATHS ${IMAGEMAGICK_BINARY_PATH}
  DOC "Path to ImageMagick's import executable."
)

FIND_PROGRAM(IMAGEMAGICK_MONTAGE_EXECUTABLE
  NAMES montage
  PATHS ${IMAGEMAGICK_BINARY_PATH}
  DOC "Path to ImageMagick's montage executable."
)

FIND_PROGRAM(IMAGEMAGICK_COMPOSITE_EXECUTABLE
  NAMES composite
  PATHS ${IMAGEMAGICK_BINARY_PATH}
  DOC "Path to ImageMagick's composite executable."
)

MARK_AS_ADVANCED(
  IMAGEMAGICK_BINARY_PATH
  IMAGEMAGICK_CONVERT_EXECUTABLE
  IMAGEMAGICK_MOGRIFY_EXECUTABLE
  IMAGEMAGICK_IMPORT_EXECUTABLE
  IMAGEMAGICK_MONTAGE_EXECUTABLE
  IMAGEMAGICK_COMPOSITE_EXECUTABLE
)
