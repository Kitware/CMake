FIND_PROGRAM(CMAKE_MAKE_PROGRAM
  NAMES VCExpress
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Setup\\VS;EnvironmentDirectory]
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Setup;Dbghelp_path]
  "c:/Program Files/Microsoft Visual Studio 8/Common7/IDE"
  "c:/Program Files/Microsoft Visual Studio8/Common7/IDE"
  "/Program Files/Microsoft Visual Studio 8/Common7/IDE/"
  )
MARK_AS_ADVANCED(CMAKE_MAKE_PROGRAM)
