#
# This module finds if Java is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  JAVA_AWT_LIB_PATH     = the path to where the jawt library is
#  JAVA_INCLUDE_PATH     = the path to where jni.h can be found
#  JAVA_AWT_INCLUDE_PATH = the path to where jni.h can be found
# 
        
FIND_LIBRARY(JAVA_AWT_LIBRARY jawt 
  PATHS /usr/lib /usr/local/lib
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.3;JavaHome]/lib"
)

# add in the include path    
FIND_PATH(JAVA_INCLUDE_PATH jni.h 
  /usr/include 
  /usr/local/include
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\1.3;JavaHome]/include"
)

FIND_PATH(JAVA_INCLUDE_PATH2 jni_md.h 
  ${JAVA_INCLUDE_PATH}/win32
  ${JAVA_INCLUDE_PATH}/linux
)

FIND_PATH(JAVE_AWT_INCLUDE_PATH jawt.h ${JAVA_INCLUDE_PATH} )
