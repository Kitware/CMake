
# hard code these for fast backwards compatibility tests
SET (CMAKE_SIZEOF_INT       4   CACHE INTERNAL "Size of int data type")
SET (CMAKE_SIZEOF_LONG      4   CACHE INTERNAL "Size of long data type")
SET (CMAKE_SIZEOF_VOID_P    4   CACHE INTERNAL "Size of void* data type")
SET (CMAKE_SIZEOF_CHAR      1   CACHE INTERNAL "Size of char data type")
SET (CMAKE_SIZEOF_SHORT     2   CACHE INTERNAL "Size of short data type")
SET (CMAKE_SIZEOF_FLOAT     4   CACHE INTERNAL "Size of float data type")
SET (CMAKE_SIZEOF_DOUBLE    8   CACHE INTERNAL "Size of double data type")
SET (CMAKE_NO_ANSI_FOR_SCOPE 0 CACHE INTERNAL 
         "Does the compiler support ansi for scope.")
SET (CMAKE_USE_WIN32_THREADS  TRUE CACHE BOOL    "Use the win32 thread library.")
SET (CMAKE_WORDS_BIGENDIAN 0 CACHE INTERNAL "endianness of bytes")
