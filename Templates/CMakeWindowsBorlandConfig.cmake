#
# Borland configuration.
# Note that this is valid for Borland C++Builder 5 with patch #1
#

SET (WORDS_BIGENDIAN )

SET (CMAKE_CXX_COMPILER  "Borland_BCB_5.5.1" CACHE STRING "C++ compiler used.")

#
# We need the Borland compiler path
#

FIND_PATH(BCB_BIN_PATH bcc32.exe
  "C:/Program Files/Borland/CBuilder5/Bin"
  "C:/Borland/Bcc55/Bin"
  "/Borland/Bcc55/Bin"
  [HKEY_LOCAL_MACHINE/SOFTWARE/Borland/C++Builder/5.0/RootDir]/Bin
)

#
# override opengl library as it is bound to import32.lib already
#

IF (OPENGL_LIBRARY)
  SET (OPENGL_LIBRARY import32 CACHE STRING "OpenGL library linked by Borland's import32.lib")
ENDIF (OPENGL_LIBRARY)

#
# Set debug compile flags if not already set/edited by user
#

IF (NOT FLAGS_CPP_DEBUG)
  SET (FLAGS_CPP_DEBUG   "-a8 -c -d -tWM -tWR -Ve -Vx -k  -Od -r- -v -vi- -y" CACHE STRING "Flags used by CPP compiler in DEBUG mode")
ENDIF (NOT FLAGS_CPP_DEBUG)

#
# Set release compile flags if not already set/edited by user
#

IF (NOT FLAGS_CPP_RELEASE)
  SET (FLAGS_CPP_RELEASE "-a8 -c -d -tWM -tWR -Ve -Vx -k- -O2 -r  -v-" CACHE STRING "Flags used by CPP compiler in RELEASE mode")
ENDIF (NOT FLAGS_CPP_RELEASE)

#
# Set compiler warning flags if not already set/edited by user
#

IF (NOT FLAGS_CPP_WARNING)
  SET (FLAGS_CPP_WARNING "-w- -whid -waus -wpar" CACHE STRING "Flags used to control compiler warnings")
ENDIF (NOT FLAGS_CPP_WARNING)

#
# Set link flags if not already set/edited by user
#

IF (NOT FLAGS_LINK_DLL)
  SET (FLAGS_LINK_DLL "-aa -Tpd -x -Gn -Gl" CACHE STRING "Flags used by Linker for DLL")
ENDIF (NOT FLAGS_LINK_DLL)

IF (NOT FLAGS_LINK_BPL)
  SET (FLAGS_LINK_BPL "-aa -Tpp -x -Gn -Gi" CACHE STRING "Flags used by Linker for BPL")
ENDIF (NOT FLAGS_LINK_BPL)

IF (NOT FLAGS_LINK_LIB)
  SET (FLAGS_LINK_LIB "-aa -x -Gn -Gl -P128" CACHE STRING "Flags used by Linker for LIB")
ENDIF (NOT FLAGS_LINK_LIB)

IF (NOT FLAGS_LINK_EXE)
  SET (FLAGS_LINK_EXE "-aa -Tpe -x -Gn" CACHE STRING "Flags used by Linker for EXE")
ENDIF (NOT FLAGS_LINK_EXE)

IF (NOT FLAGS_LINK_DEBUG)
  SET (FLAGS_LINK_DEBUG "-v" CACHE STRING "Flags used by Linker in DEBUG mode")
ENDIF (NOT FLAGS_LINK_DEBUG)

IF (NOT FLAGS_LINK_STATIC)
  SET (FLAGS_LINK_STATIC "/P128" CACHE STRING "Set default Page size to 128 for static libraries")
ENDIF (NOT FLAGS_LINK_STATIC)

#
# Set User Conditional Defines to Defaults
#

IF (NOT DEFS_USER)
  SET (DEFS_USER "" CACHE STRING "Compiler conditional defines set by the user")
ENDIF (NOT DEFS_USER)

#
# Set SYS Conditional Defines to Defaults
#

IF (NOT DEFS_SYS)
  SET (DEFS_SYS "-DWIN32;WIN32_LEAN_AND_MEAN;STRICT;_RTLDLL;USEPACKAGES" CACHE STRING "Compiler conditional defines required for correct compilation")
ENDIF (NOT DEFS_SYS)

FIND_PROGRAM(CMAKE_MAKE_PROGRAM make ${BCB_BIN_PATH} )
