FIND_PROGRAM(CMAKE_MAKE_PROGRAM
  NAMES msdev
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\6.0\\Setup;VsCommonDir]/MSDev98/Bin
  "c:/Program Files/Microsoft Visual Studio/Common/MSDev98/Bin"
  "c:/Program Files/Microsoft Visual Studio/Common/MSDev98/Bin"
  "/Program Files/Microsoft Visual Studio/Common/MSDev98/Bin"
  )
MARK_AS_ADVANCED(CMAKE_MAKE_PROGRAM)
SET(MSVC60 1)
