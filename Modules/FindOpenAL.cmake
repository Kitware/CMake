# Locate OpenAL
# This module defines
# OPENAL_LIBRARY
# OPENAL_FOUND, if false, do not try to link to OpenAL 
# OPENAL_INCLUDE_DIR, where to find the headers
#
# $OPENALDIR is an environment variable that would
# correspond to the ./configure --prefix=$OPENALDIR
# used in building OpenAL.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake 
# module, but with modifications to recognize OS X frameworks.

# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of 
# OPENAL_LIBRARY to override this selection.
# Tiger will include OpenAL as part of the System.
# But for now, we have to look around.
# Other (Unix) systems should be able to utilize the non-framework paths.
FIND_PATH(OPENAL_INCLUDE_DIR al.h
  ~/Library/Frameworks/OpenAL.framework/Headers
  /Library/Frameworks/OpenAL.framework/Headers
  /System/Library/Frameworks/OpenAL.framework/Headers
  $ENV{OPENALDIR}/include
  /usr/include
  /usr/include/AL
  /usr/include/OpenAL
  /usr/local/include/AL
  /usr/local/include/OpenAL
  /sw/include
  /sw/include/AL
  )
# I'm not sure if I should do a special casing for Apple. It is 
# unlikely that other Unix systems will find the framework path.
# But if they do ([Next|Open|GNU]Step?), 
# do they want the -framework option also?
IF(${OPENAL_INCLUDE_DIR} MATCHES ".framework")
  SET (OPENAL_LIBRARY "-framework OpenAL" CACHE STRING "OpenAL framework for OSX")
ELSE(${OPENAL_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(OPENAL_LIBRARY 
    NAMES openal al
    PATHS
    $ENV{OPENALDIR}/lib
    /usr/lib
    /usr/local/lib
    /sw/lib
    )
ENDIF(${OPENAL_INCLUDE_DIR} MATCHES ".framework")

SET(OPENAL_FOUND "NO")
IF(OPENAL_LIBRARY)
  SET(OPENAL_FOUND "YES")
ENDIF(OPENAL_LIBRARY)


