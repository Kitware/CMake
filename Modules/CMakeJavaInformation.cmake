# this is a place holder if java needed flags for javac they would go here.
SET(CMAKE_STATIC_LIBRARY_SUFFIX_Java ".jar")
IF(NOT CMAKE_Java_CREATE_STATIC_LIBRARY)
  SET(CMAKE_Java_CREATE_STATIC_LIBRARY
      "<CMAKE_Java_ARCHIVE> -cf <TARGET> *.class") 
# should be this <OBJECTS> but compling a java file can create more than one .class file
# so for now get all of them
ENDIF(NOT CMAKE_Java_CREATE_STATIC_LIBRARY)
# compile a Java file into an object file
IF(NOT CMAKE_Java_COMPILE_OBJECT)
  SET(CMAKE_Java_COMPILE_OBJECT
    "<CMAKE_Java_COMPILER>   <FLAGS> <SOURCE> -d <CMAKE_CURRENT_BINARY_DIR>")
ENDIF(NOT CMAKE_Java_COMPILE_OBJECT)

# set java include flag option and the separator for multiple include paths
SET(CMAKE_INCLUDE_FLAG_Java "-classpath ")
IF(WIN32 AND NOT CYGWIN)
  SET(CMAKE_INCLUDE_FLAG_SEP_Java ";")
ELSE(WIN32 AND NOT CYGWIN)
  SET(CMAKE_INCLUDE_FLAG_SEP_Java ":")
ENDIF(WIN32 AND NOT CYGWIN)
