# - try to find GTK (and glib) and GTKGLArea
#  GTK_INCLUDE_DIR   - Directories to include to use GTK
#  GTK_LIBRARIES     - Files to link against to use GTK
#  GTK_FOUND         - GTK was found
#  GTK_GL_FOUND      - GTK's GL features were found

# don't even bother under WIN32
IF(UNIX)

  FIND_PATH( GTK_gtk_INCLUDE_PATH gtk/gtk.h
    /usr/include
    /usr/local/include
    /usr/openwin/share/include
    /usr/openwin/include
    /usr/X11R6/include
    /usr/include/X11
    /usr/X11R6/include/gtk12
    /usr/include/gtk-1.2
    /usr/local/include/gtk-1.2
    /opt/gnome/include
  )

  # Some Linux distributions (e.g. Red Hat) have glibconfig.h
  # and glib.h in different directories, so we need to look
  # for both.
  #  - Atanas Georgiev <atanas@cs.columbia.edu>

  FIND_PATH( GTK_glibconfig_INCLUDE_PATH glibconfig.h
    /usr/include
    /usr/local/include
    /usr/openwin/share/include
    /usr/local/include/glib12
    /usr/lib/glib/include
    /usr/local/lib/glib/include
    /opt/gnome/include
    /opt/gnome/lib/glib/include
  )

  FIND_PATH( GTK_glib_INCLUDE_PATH glib.h
    /usr/include
    /usr/local/include
    /usr/openwin/share/include
    /usr/include/gtk-1.2
    /usr/local/include/glib12
    /usr/lib/glib/include
    /usr/include/glib-1.2
    /usr/local/include/glib-1.2
    /opt/gnome/include
    /opt/gnome/include/glib-1.2
  )

  FIND_PATH( GTK_gtkgl_INCLUDE_PATH gtkgl/gtkglarea.h
    /usr/include
    /usr/local/include
    /usr/openwin/share/include
    /opt/gnome/include
  )

  FIND_LIBRARY( GTK_gtkgl_LIBRARY gtkgl
    /usr/lib
    /usr/local/lib
    /usr/openwin/lib
    /usr/X11R6/lib
    /opt/gnome/lib
  )

  #
  # The 12 suffix is thanks to the FreeBSD ports collection
  #

  FIND_LIBRARY( GTK_gtk_LIBRARY
    NAMES  gtk gtk12
    PATHS /usr/lib
          /usr/local/lib
          /usr/openwin/lib
          /usr/X11R6/lib
          /opt/gnome/lib
  )

  FIND_LIBRARY( GTK_gdk_LIBRARY
    NAMES  gdk gdk12
    PATHS  /usr/lib
           /usr/local/lib
           /usr/openwin/lib
           /usr/X11R6/lib
           /opt/gnome/lib
  )

  FIND_LIBRARY( GTK_gmodule_LIBRARY
    NAMES  gmodule gmodule12
    PATHS  /usr/lib
           /usr/local/lib
           /usr/openwin/lib
           /usr/X11R6/lib
           /opt/gnome/lib
  )

  FIND_LIBRARY( GTK_glib_LIBRARY
    NAMES  glib glib12
    PATHS  /usr/lib
           /usr/local/lib
           /usr/openwin/lib
           /usr/X11R6/lib
           /opt/gnome/lib
  )

  FIND_LIBRARY( GTK_Xi_LIBRARY 
    NAMES Xi 
    PATHS /usr/lib 
    /usr/local/lib 
    /usr/openwin/lib 
    /usr/X11R6/lib 
    /opt/gnome/lib 
    ) 

  FIND_LIBRARY( GTK_gthread_LIBRARY
    NAMES  gthread gthread12
    PATHS  /usr/lib
           /usr/local/lib
           /usr/openwin/lib
           /usr/X11R6/lib
           /opt/gnome/lib
  )

  IF(GTK_gtk_INCLUDE_PATH)
  IF(GTK_glibconfig_INCLUDE_PATH)
  IF(GTK_glib_INCLUDE_PATH)
  IF(GTK_gtk_LIBRARY)
  IF(GTK_glib_LIBRARY)

    # Assume that if gtk and glib were found, the other
    # supporting libraries have also been found.

    SET( GTK_FOUND "YES" )
    SET( GTK_INCLUDE_DIR  ${GTK_gtk_INCLUDE_PATH}
                           ${GTK_glibconfig_INCLUDE_PATH}
                           ${GTK_glib_INCLUDE_PATH} )
    SET( GTK_LIBRARIES  ${GTK_gtk_LIBRARY}
                        ${GTK_gdk_LIBRARY}
                        ${GTK_glib_LIBRARY} )

    IF(GTK_gmodule_LIBRARY)
      SET(GTK_LIBRARIES ${GTK_LIBRARIES} ${GTK_gmodule_LIBRARY})
    ENDIF(GTK_gmodule_LIBRARY)
    IF(GTK_gthread_LIBRARY)
      SET(GTK_LIBRARIES ${GTK_LIBRARIES} ${GTK_gthread_LIBRARY})
    ENDIF(GTK_gthread_LIBRARY)
    IF(GTK_Xi_LIBRARY)
      SET(GTK_LIBRARIES ${GTK_LIBRARIES} ${GTK_Xi_LIBRARY})
    ENDIF(GTK_Xi_LIBRARY)

  IF(GTK_gtkgl_INCLUDE_PATH)
  IF(GTK_gtkgl_LIBRARY)
    SET( GTK_GL_FOUND "YES" )
    SET( GTK_INCLUDE_DIR  ${GTK_INCLUDE_DIR}
                           ${GTK_gtkgl_INCLUDE_PATH} )
    SET( GTK_LIBRARIES  ${GTK_gtkgl_LIBRARY} ${GTK_LIBRARIES} )
    MARK_AS_ADVANCED(
      GTK_gtkgl_LIBRARY
      GTK_gtkgl_INCLUDE_PATH
      )
  ENDIF(GTK_gtkgl_LIBRARY)
  ENDIF(GTK_gtkgl_INCLUDE_PATH)

  ENDIF(GTK_glib_LIBRARY)
  ENDIF(GTK_gtk_LIBRARY)
  ENDIF(GTK_glib_INCLUDE_PATH) 
  ENDIF(GTK_glibconfig_INCLUDE_PATH)
  ENDIF(GTK_gtk_INCLUDE_PATH)

  MARK_AS_ADVANCED(
    GTK_gdk_LIBRARY
    GTK_glib_INCLUDE_PATH
    GTK_glib_LIBRARY
    GTK_glibconfig_INCLUDE_PATH
    GTK_gmodule_LIBRARY
    GTK_gthread_LIBRARY
    GTK_Xi_LIBRARY
    GTK_gtk_INCLUDE_PATH
    GTK_gtk_LIBRARY
    GTK_gtkgl_INCLUDE_PATH
    GTK_gtkgl_LIBRARY
  )

ENDIF(UNIX)

