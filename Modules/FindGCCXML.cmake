FIND_PROGRAM(GCCXML gccxml
             /usr/local/bin
             /usr/bin
             [HKEY_CURRENT_USER\\Software\\Kitware\\GCC_XML;loc]
             "C:/Program Files/GCC_XML")

IF(GCCXML)
  CONFIGURE_GCCXML(${GCCXML} GCCXML_FLAGS)
ENDIF(GCCXML)
