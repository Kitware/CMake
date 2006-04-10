FIND_PROGRAM(CMAKE_MAKE_PROGRAM
  NAMES devenv
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\7.0\\Setup\\VS;EnvironmentDirectory]
  "c:/Program Files/Microsoft Visual Studio .NET/Common7/IDE"
  "c:/Program Files/Microsoft Visual Studio.NET/Common7/IDE"
  "/Program Files/Microsoft Visual Studio .NET/Common7/IDE/"
  )
MARK_AS_ADVANCED(CMAKE_MAKE_PROGRAM)
SET(MSVC70 1)
