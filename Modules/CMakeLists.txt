# just install the modules
SUBDIRS(Platform)
INSTALL_FILES(/share/CMake/Modules .*\\.cmake$)
INSTALL_FILES(/share/CMake/Modules .*\\.cxx$)
INSTALL_FILES(/share/CMake/Modules .*\\.in$)
INSTALL_FILES(/share/CMake/Modules .*\\.c$)

