# just install the modules
INSTALL_PROGRAMS(${CMAKE_DATA_DIR}/Templates install-sh)
INSTALL_FILES(${CMAKE_DATA_DIR}/Templates .*\\.cmake$)
INSTALL_FILES(${CMAKE_DATA_DIR}/Templates .*\\.dsptemplate$)

