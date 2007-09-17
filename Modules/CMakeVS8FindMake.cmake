FIND_PROGRAM(CMAKE_MAKE_PROGRAM
  NAMES VCExpress devenv 
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Setup\\VS;EnvironmentDirectory]
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Setup;Dbghelp_path]
  "$ENV{ProgramFiles}/Microsoft Visual Studio .NET/Common7/IDE"
  "$ENV{ProgramFiles}/Microsoft Visual Studio 8/Common7/IDE"
  "$ENV{ProgramFiles}/Microsoft Visual Studio8/Common7/IDE"
  "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio .NET/Common7/IDE"
  "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio 8/Common7/IDE"
  "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio8/Common7/IDE"
  "/Program Files/Microsoft Visual Studio 8/Common7/IDE/"
  )
MARK_AS_ADVANCED(CMAKE_MAKE_PROGRAM)
SET(MSVC80 1)
SET(MSVC_VERSION 1400)
