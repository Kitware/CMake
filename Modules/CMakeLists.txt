# just install the modules
# new file added, force rerunning cmake 

SUBDIRS(Platform)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules .*\\.cmake$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules .*\\.cpp$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules .*\\.cxx$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules .*\\.in$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules .*\\.c$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules .*\\.h$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules .*\\.F90$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Modules readme\\.txt$)
