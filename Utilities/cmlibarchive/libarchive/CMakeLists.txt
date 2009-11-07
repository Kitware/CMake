
############################################
#
# How to build libarchive
#
############################################

# Public headers
SET(include_HEADERS
  archive.h
  archive_entry.h
)

# Sources and private headers
SET(libarchive_SOURCES
  archive_check_magic.c
  archive_endian.h
  archive_entry.c
  archive_entry.h
  archive_entry_copy_stat.c
#  archive_entry_copy_bhfi.c
  archive_entry_link_resolver.c
  archive_entry_private.h
  archive_entry_stat.c
  archive_entry_strmode.c
  archive_entry_xattr.c
  archive_platform.h
  archive_private.h
  archive_read.c
  archive_read_data_into_fd.c
  archive_read_disk.c
  archive_read_disk_entry_from_file.c
  archive_read_disk_private.h
  archive_read_disk_set_standard_lookup.c
  archive_read_extract.c
  archive_read_open_fd.c
  archive_read_open_file.c
  archive_read_open_filename.c
  archive_read_open_memory.c
  archive_read_private.h
  archive_read_support_compression_all.c
  archive_read_support_compression_bzip2.c
  archive_read_support_compression_compress.c
  archive_read_support_compression_gzip.c
  archive_read_support_compression_none.c
  archive_read_support_compression_program.c
  archive_read_support_compression_xz.c
  archive_read_support_format_all.c
  archive_read_support_format_ar.c
  archive_read_support_format_cpio.c
  archive_read_support_format_empty.c
  archive_read_support_format_iso9660.c
  archive_read_support_format_mtree.c
  archive_read_support_format_raw.c
  archive_read_support_format_tar.c
  archive_read_support_format_zip.c
  archive_string.c
  archive_string.h
  archive_string_sprintf.c
  archive_util.c
  archive_virtual.c
  archive_write.c
  archive_write_disk.c
  archive_write_disk_private.h
  archive_write_disk_set_standard_lookup.c
  archive_write_private.h
  archive_write_open_fd.c
  archive_write_open_file.c
  archive_write_open_filename.c
  archive_write_open_memory.c
  archive_write_set_compression_bzip2.c
  archive_write_set_compression_compress.c
  archive_write_set_compression_gzip.c
  archive_write_set_compression_none.c
  archive_write_set_compression_program.c
  archive_write_set_compression_xz.c
  archive_write_set_format.c
  archive_write_set_format_ar.c
  archive_write_set_format_by_name.c
  archive_write_set_format_cpio.c
  archive_write_set_format_cpio_newc.c
  archive_write_set_format_mtree.c
  archive_write_set_format_pax.c
  archive_write_set_format_shar.c
  archive_write_set_format_ustar.c
  archive_write_set_format_zip.c
  filter_fork.c
  filter_fork.h
)

# Man pages
SET(libarchive_MANS
  archive_entry.3
  archive_read.3
  archive_read_disk.3
  archive_util.3
  archive_write.3
  archive_write_disk.3
  cpio.5
  libarchive.3
  libarchive_internals.3
  libarchive-formats.5
  mtree.5
  tar.5
)

IF(WIN32 AND NOT CYGWIN)
  LIST(APPEND libarchive_SOURCES archive_entry_copy_bhfi.c)
  LIST(APPEND libarchive_SOURCES archive_windows.c)
  LIST(APPEND libarchive_SOURCES archive_windows.h)
  LIST(APPEND libarchive_SOURCES filter_fork_windows.c)
ENDIF(WIN32 AND NOT CYGWIN)

IF(BUILD_ARCHIVE_WITHIN_CMAKE)
  # when building inside the CMake tree only use static
  # and call the library cmlibarchive
  ADD_LIBRARY(cmlibarchive STATIC ${libarchive_SOURCES} ${include_HEADERS})
  TARGET_LINK_LIBRARIES(cmlibarchive ${ADDITIONAL_LIBS})
ELSE()
  # Libarchive is a shared library
  ADD_LIBRARY(archive SHARED ${libarchive_SOURCES} ${include_HEADERS})
  TARGET_LINK_LIBRARIES(archive ${ADDITIONAL_LIBS})
  SET_TARGET_PROPERTIES(archive PROPERTIES SOVERSION ${SOVERSION})
  SET_TARGET_PROPERTIES(archive PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  
  # archive_static is a static library
  ADD_LIBRARY(archive_static STATIC ${libarchive_SOURCES} ${include_HEADERS})
  SET_TARGET_PROPERTIES(archive_static PROPERTIES COMPILE_DEFINITIONS
    LIBARCHIVE_STATIC)
  SET_TARGET_PROPERTIES(archive_static PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  #  On Posix systems, libarchive.so and libarchive.a can co-exist.
  IF(NOT WIN32 OR CYGWIN)
    SET_TARGET_PROPERTIES(archive_static PROPERTIES OUTPUT_NAME archive)
  ENDIF(NOT WIN32 OR CYGWIN)
  
  # How to install the libraries
  INSTALL(TARGETS archive archive_static
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  INSTALL_MAN(${libarchive_MANS})
  INSTALL(FILES ${include_HEADERS} DESTINATION include)
  ADD_SUBDIRECTORY(test)
ENDIF()
