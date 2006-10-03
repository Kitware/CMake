# - Locate the SDL library
# This module defines
#  SDL_LIBRARY, the library to link against
#  SDL_FOUND, if false, do not try to link to SDL
#  SDL_INCLUDE_DIR, where to find SDL.h
#
# Don't forget to include SDLmain.h and SDLmain.m your project for the 
# OS X framework based version. (Other versions link to -lSDLmain which
# this module will try to find on your behalf.) Also for OS X, this 
# module will automatically add the -framework Cocoa on your behalf.
# $SDLDIR is an environment variable that would
# correspond to the ./configure --prefix=$SDLDIR
# used in building SDL.
# l.e.galup  9-20-02
#
# Modified by Eric Wing. 
# Added new modifications to recognize OS X frameworks and 
# additional Unix paths (FreeBSD, etc). 
# Also corrected the header search path to follow "proper" SDL guidelines.
# Added a search for SDLmain which is needed by some platforms.
# Added a search for threads which is needed by some platforms.
# Added needed compile switches for MinGW.
#
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of 
# SDL_LIBRARY to override this selection.
#
# Note that the header path has changed from SDL/SDL.h to just SDL.h
# This needed to change because "proper" SDL convention
# is #include "SDL.h", not <SDL/SDL.h>. This is done for portability
# reasons because not all systems place things in SDL/ (see FreeBSD).
FIND_PATH(SDL_INCLUDE_DIR SDL.h
  $ENV{SDLDIR}/include
  ~/Library/Frameworks/SDL.framework/Headers
  /Library/Frameworks/SDL.framework/Headers
  /usr/local/include/SDL
  /usr/include/SDL
  /usr/local/include/SDL12
  /usr/local/include/SDL11 # FreeBSD ports
  /usr/include/SDL12
  /usr/include/SDL11
  /usr/local/include
  /usr/include
  /sw/include/SDL # Fink
  /sw/include
  /opt/local/include/SDL # DarwinPorts
  /opt/local/include
  /opt/csw/include/SDL # Blastwave
  /opt/csw/include 
  /opt/include/SDL
  /opt/include
  )
# I'm not sure if I should do a special casing for Apple. It is 
# unlikely that other Unix systems will find the framework path.
# But if they do ([Next|Open|GNU]Step?), 
# do they want the -framework option also?
IF(${SDL_INCLUDE_DIR} MATCHES ".framework")
  # The Cocoa framework must be linked into SDL because SDL is Cocoa based.
  # Remember that the OS X framework version expects you to drop in
  # SDLmain.h and SDLmain.m directly into your project.
  # (Cocoa link moved to bottom of this script.)
  # SET (SDL_LIBRARY "-framework SDL -framework Cocoa" CACHE STRING "SDL framework for OSX")
  # SET(SDL_LIBRARY "-framework SDL" CACHE STRING "SDL framework for OSX")
  # Extract the path the framework resides in so we can use it for the -F flag
  STRING(REGEX REPLACE "(.*)/.*\\.framework/.*" "\\1" SDL_FRAMEWORK_PATH_TEMP ${SDL_INCLUDE_DIR})
  IF("${SDL_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDL_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is in default search path, don't need to use -F
    SET(SDL_LIBRARY_TEMP "-framework SDL")
  ELSE("${SDL_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDL_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is not /Library/Frameworks, need to use -F
    SET(SDL_LIBRARY_TEMP "-F${SDL_FRAMEWORK_PATH_TEMP} -framework SDL")
  ENDIF("${SDL_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
    OR "${SDL_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
    )
  # Clear the temp variable so nobody can see it
  SET(SDL_FRAMEWORK_PATH_TEMP "" CACHE INTERNAL "")

ELSE(${SDL_INCLUDE_DIR} MATCHES ".framework")
  # SDL-1.1 is the name used by FreeBSD ports...
  # don't confuse it for the version number.
  FIND_LIBRARY(SDL_LIBRARY_TEMP 
    NAMES SDL SDL-1.1
    PATHS
    $ENV{SDLDIR}/lib
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    )
  # Non-OS X framework versions expect you to also dynamically link to 
  # SDLmain. This is mainly for Windows and OS X. Other platforms 
  # seem to provide SDLmain for compatibility even though they don't
  # necessarily need it.
  FIND_LIBRARY(SDLMAIN_LIBRARY 
    NAMES SDLmain SDLmain-1.1
    PATHS
    $ENV{SDLDIR}/lib
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    )
ENDIF(${SDL_INCLUDE_DIR} MATCHES ".framework")

# SDL may require threads on your system.
# The Apple build may not need an explicit flag because one of the 
# frameworks may already provide it. 
# But for non-OSX systems, I will use the CMake Threads package.
IF(NOT APPLE)
  FIND_PACKAGE(Threads)
ENDIF(NOT APPLE)

# MinGW needs an additional library, mwindows
# It's total link flags should look like -lmingw32 -lSDLmain -lSDL -lmwindows
# (Actually on second look, I think it only needs one of the m* libraries.)
IF(MINGW)
  SET(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
ENDIF(MINGW)

SET(SDL_FOUND "NO")
IF(SDL_LIBRARY_TEMP)
  # For SDLmain
  IF(SDLMAIN_LIBRARY)
    SET(SDL_LIBRARY_TEMP ${SDLMAIN_LIBRARY} ${SDL_LIBRARY_TEMP})
  ENDIF(SDLMAIN_LIBRARY)

  # For OS X, SDL uses Cocoa as a backend so it must link to Cocoa.
  # CMake doesn't display the -framework Cocoa string in the UI even 
  # though it actually is there. I think it has something to do 
  # with the CACHE STRING. Maybe somebody else knows how to fix this.
  # The problem is mainly cosmetic, and not a functional issue.
  IF(APPLE)
    SET(SDL_LIBRARY_TEMP ${SDL_LIBRARY_TEMP} "-framework Cocoa")
  ENDIF(APPLE)
  
  # For threads, as mentioned Apple doesn't need this.
  # In fact, there seems to be a problem if Find the threads package
  # and try using this line, so I'm just skipping it entirely for OS X.
  IF(NOT APPLE)
    SET(SDL_LIBRARY_TEMP ${SDL_LIBRARY_TEMP} ${CMAKE_THREAD_LIBS_INIT})
  ENDIF(NOT APPLE)

  # For MinGW library
  IF(MINGW)
    SET(SDL_LIBRARY_TEMP ${MINGW32_LIBRARY} ${SDL_LIBRARY_TEMP})
  ENDIF(MINGW)

  # Set the final string here so the GUI reflects the final state.
  SET(SDL_LIBRARY ${SDL_LIBRARY_TEMP} CACHE STRING "Where the SDL Library can be found")

  SET(SDL_FOUND "YES")
ENDIF(SDL_LIBRARY_TEMP)

MARK_AS_ADVANCED(SDL_LIBRARY_TEMP)
