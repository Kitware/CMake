############################################
#
# How to build bsdcpio
#
############################################
if(ENABLE_CPIO)

  set(bsdcpio_SOURCES
    cmdline.c
    cpio.c
    cpio.h
    cpio_platform.h
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
    list(APPEND bsdcpio_SOURCES cpio_windows.c)
    list(APPEND bsdcpio_SOURCES cpio_windows.h)
  endif(WIN32 AND NOT CYGWIN)

  # bsdcpio documentation
  set(bsdcpio_MANS bsdcpio.1)

  # How to build bsdcpio
  add_executable(bsdcpio ${bsdcpio_SOURCES})
  if(ENABLE_CPIO_SHARED)
    target_link_libraries(bsdcpio archive ${ADDITIONAL_LIBS})
  else(ENABLE_CPIO_SHARED)
    target_link_libraries(bsdcpio archive_static ${ADDITIONAL_LIBS})
  endif(ENABLE_CPIO_SHARED)
  # On Windows, DLL must end up in same dir with EXEs
  if(WIN32 AND NOT CYGWIN)
    set_target_properties(bsdcpio PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  endif(WIN32 AND NOT CYGWIN)
  # Full path to the compiled executable (used by test suite)
  get_target_property(BSDCPIO bsdcpio LOCATION)

  # Installation rules
  install(TARGETS bsdcpio RUNTIME DESTINATION bin)
  INSTALL_MAN(${bsdcpio_MANS})

endif(ENABLE_CPIO)

# Test suite
add_subdirectory(test)
