############################################
#
# How to build bsdtar
#
############################################
if(ENABLE_TAR)

  set(bsdtar_SOURCES
    bsdtar.c
    bsdtar.h
    bsdtar_platform.h
    cmdline.c
    getdate.c
    read.c
    subst.c
    tree.c
    tree.h
    util.c
    write.c
    ../libarchive_fe/err.c
    ../libarchive_fe/err.h
    ../libarchive_fe/lafe_platform.h
    ../libarchive_fe/line_reader.c
    ../libarchive_fe/line_reader.h
    ../libarchive_fe/matching.c
    ../libarchive_fe/matching.h
    ../libarchive_fe/pathmatch.c
    ../libarchive_fe/pathmatch.h
  )
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libarchive_fe)
  if(WIN32 AND NOT CYGWIN)
    list(APPEND bsdtar_SOURCES bsdtar_windows.c)
    list(APPEND bsdtar_SOURCES bsdtar_windows.h)
  endif(WIN32 AND NOT CYGWIN)

  # bsdtar documentation
  set(bsdtar_MANS bsdtar.1)

  # How to build bsdtar
  add_executable(bsdtar ${bsdtar_SOURCES})
  if(ENABLE_TAR_SHARED)
    target_link_libraries(bsdtar archive ${ADDITIONAL_LIBS})
  else(ENABLE_TAR_SHARED)
    target_link_libraries(bsdtar archive_static ${ADDITIONAL_LIBS})
  endif(ENABLE_TAR_SHARED)
  # On Windows, DLL must end up in same dir with EXEs
  if(WIN32 AND NOT CYGWIN)
    set_target_properties(bsdtar PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  endif(WIN32 AND NOT CYGWIN)
  get_target_property(BSDTAR bsdtar LOCATION)

  # Installation rules
  install(TARGETS bsdtar RUNTIME DESTINATION bin)
  INSTALL_MAN(${bsdtar_MANS})
endif(ENABLE_TAR)

add_subdirectory(test)
