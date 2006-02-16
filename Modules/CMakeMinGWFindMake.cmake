FIND_PROGRAM(CMAKE_MAKE_PROGRAM mingw32-make.exe PATHS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MinGW;InstallLocation]/bin" 
  c:/MinGW/bin /MinGW/bin)
FIND_PROGRAM(CMAKE_SH sh.exe )
IF(CMAKE_SH)
  MESSAGE(FATAL_ERROR "sh.exe was found in your PATH, here:\n${CMAKE_SH}\nFor MinGW make to work correctly sh.exe must NOT be in your path.\nRun cmake from a shell that does not have sh.exe in your PATH.\nIf you want to use a UNIX shell, then use MSYS Makefiles.\n")
  SET(CMAKE_MAKE_PROGRAM NOTFOUND)
ENDIF(CMAKE_SH)

MARK_AS_ADVANCED(CMAKE_MAKE_PROGRAM CMAKE_SH)
