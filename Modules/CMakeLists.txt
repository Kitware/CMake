# just install the modules
SUBDIRS(Platform)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules .*\\.cmake$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules .*\\.cxx$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules .*\\.in$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules .*\\.c$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules readme\\.txt$)
