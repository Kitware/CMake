FIND_PROGRAM(GCCXML
  NAMES gccxml
        ../share/GCC_XML/gccxml
        ../GCC_XML/gccxml
  PATHS /usr/share/GCC_XML
        /usr/local/share/GCC_XML
        [HKEY_CURRENT_USER\\Software\\Kitware\\GCC_XML;loc]
        "C:/Program Files/GCC_XML"
)

IF(GCCXML)
  CONFIGURE_GCCXML(${GCCXML} GCCXML_FLAGS)
ENDIF(GCCXML)
