#
# This module finds if Java is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  JAVA_AWT_LIB_PATH     = the path to where the jawt library is
#  JAVA_INCLUDE_PATH     = the path to where jni.h can be found
#  JAVA_AWT_INCLUDE_PATH = the path to where jni.h can be found
# 

SET(JAVA_AWT_LIBRARY_DIRECTORIES
  /usr/lib
  /usr/local/lib
  /usr/lib/java/jre/lib/i386
  /usr/local/lib/java/jre/lib/i386
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.4;JavaHome]/lib"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.3;JavaHome]/lib"
  )

SET(JAVA_AWT_INCLUDE_DIRECTORIES
  /usr/include 
  /usr/local/include
  /usr/lib/java/include
  /usr/local/lib/java/include
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.4;JavaHome]/include"
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.3;JavaHome]/include"
  )

FIND_LIBRARY(JAVA_AWT_LIBRARY jawt 
  PATHS ${JAVA_AWT_LIBRARY_DIRECTORIES}
)

IF(APPLE)
  IF(EXISTS ~/Library/Frameworks/JavaEmbedding.framework)
    SET(JAVA_HAVE_FRAMEWORK 1)
  ENDIF(EXISTS ~/Library/Frameworks/JavaEmbedding.framework)
  IF(EXISTS /Library/Frameworks/JavaEmbedding.framework)
    SET(JAVA_HAVE_FRAMEWORK 1)
  ENDIF(EXISTS /Library/Frameworks/JavaEmbedding.framework)
  IF(JAVA_HAVE_FRAMEWORK)
    IF(NOT JAVA_LIBRARY)
      SET (JAVA_LIBRARY "-framework JavaVM -framework JavaEmbedding" CACHE FILEPATH "Java Frameworks" FORCE)
      SET(JAVA_AWT_INCLUDE_DIRECTORIES
        ~/Library/Frameworks/JavaEmbedding.framework/Headers
        /Library/Frameworks/JavaEmbedding.framework/Headers
        /System/Library/Frameworks/JavaEmbedding.framework/Headers
        )
    ENDIF(NOT JAVA_LIBRARY)
  ENDIF(JAVA_HAVE_FRAMEWORK)
ENDIF(APPLE)

# add in the include path    
FIND_PATH(JAVA_INCLUDE_PATH jni.h 
  ${JAVA_AWT_INCLUDE_DIRECTORIES}
)

FIND_PATH(JAVA_INCLUDE_PATH2 jni_md.h 
  ${JAVA_AWT_INCLUDE_DIRECTORIES}
  ${JAVA_INCLUDE_PATH}/win32
  ${JAVA_INCLUDE_PATH}/linux
)

FIND_PATH(JAVA_AWT_INCLUDE_PATH jawt.h ${JAVA_INCLUDE_PATH} )

MARK_AS_ADVANCED(
  JAVA_AWT_LIBRARY
  JAVA_AWT_INCLUDE_PATH
  JAVA_INCLUDE_PATH
  JAVA_INCLUDE_PATH2
)
