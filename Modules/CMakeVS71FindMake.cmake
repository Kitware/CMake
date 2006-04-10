FIND_PROGRAM(CMAKE_MAKE_PROGRAM
  NAMES devenv
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\7.1\\Setup\\VS;EnvironmentDirectory]
  "$ENV{ProgramFiles}/Microsoft Visual Studio .NET/Common7/IDE"
  "c:/Program Files/Microsoft Visual Studio .NET/Common7/IDE"
  "c:/Program Files/Microsoft Visual Studio.NET/Common7/IDE"
  "/Program Files/Microsoft Visual Studio .NET/Common7/IDE/"
  )
MARK_AS_ADVANCED(CMAKE_MAKE_PROGRAM)
SET(MSVC71 1)
