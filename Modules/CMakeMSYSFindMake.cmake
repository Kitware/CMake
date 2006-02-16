FIND_PROGRAM(CMAKE_MAKE_PROGRAM make 
  PATHS 
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MSYS-1.0_is1;Inno Setup: App Path]/bin"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MinGW;InstallLocation]/bin"
  c:/msys/1.0/bin /msys/1.0/bin)
MARK_AS_ADVANCED(CMAKE_MAKE_PROGRAM)
