#
# try to find GTK
#

# don't even bother under WIN32
IF (UNIX)

  FIND_PATH(GTK_INCLUDE_PATH gtk/gtk.h
  /usr/include
  /usr/local/include
  /usr/openwin/share/include
  )

  FIND_LIBRARY(GTK_LIB_PATH  gtk
  PATHS /usr/lib  /usr/local/lib  /usr/openwin/lib  /usr/X11R6/lib
  )

  FIND_PATH(GTK_GLIB_INCLUDE_PATH glibconfig.h
  /usr/include
  /usr/local/include
  /usr/openwin/share/include
  )

ENDIF (UNIX)

