# Find the GCC-XML front-end executable.
FIND_PROGRAM(GCCXML
  NAMES gccxml
        ../GCC_XML/gccxml
  PATHS [HKEY_CURRENT_USER\\Software\\Kitware\\GCC_XML;loc]
        "C:/Program Files/GCC_XML"
)
