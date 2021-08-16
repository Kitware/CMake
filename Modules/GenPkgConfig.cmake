#[=======================================================================[.rst:

GenPkgConfig
------------

This is the library helping you to generate and install pkg-config files.

Functions
^^^^^^^^^
.. command:: configure_pkg_config_file

  .. versionadded:: 3.22

  Generates a pkg-config file for

  ::

    configure_pkg_config_file(<targetName>
        NAME <name of the package>
        VERSION <version to be written into the package>
        DESCRIPTION <description to be written into the package>
        URL <homepage URL to be written into the package>
        COMPONENT <install as the component>
        INSTALL_LIB_DIR <path to something like CMAKE_INSTALL_LIBDIR>
        INSTALL_INCLUDE_DIR <path to something like CMAKE_INSTALL_INCLUDEDIR>
        REQUIRES ... <list of pkg-config packages this one depends on> ...
        REQUIRES ... <list of pkg-config packages this one conflicts with> ...
    )

    The arguments are optional and usually are not needed to be set if global (not component-specific) CPACK vars have been set before.

    Generation is done in build time using packaging expressions.

#]=======================================================================]


function(configure_pkg_config_file TARGET)
  cmake_parse_arguments(""
    "" # options
    "NAME;VERSION;DESCRIPTION;URL;COMPONENT;INSTALL_LIB_DIR;INSTALL_INCLUDE_DIR" # one_value_keywords
    "REQUIRES;CONFLICTS" # multi_value_keywords
    ${ARGN}
  )

  configure_pkg_config_file_vars("${TARGET}" "${_NAME}" "${_INSTALL_LIB_DIR}" "${_INSTALL_INCLUDE_DIR}" "${_COMPONENT}" "${_DESCRIPTION}" "${_URL}" "${_VERSION}" "${_REQUIRES}" "${_CONFLICTS}")
endfunction()

function(configure_pkg_config_file_vars TARGET _NAME _INSTALL_LIB_DIR _INSTALL_INCLUDE_DIR _COMPONENT _DESCRIPTION _URL _VERSION _REQUIRES _CONFLICTS)
  #$<TARGET_PROPERTY:${TARGET},NAME>
  #INTERFACE_LINK_DIRECTORIES
  #INTERFACE_LINK_LIBRARIES
  #INTERFACE_LINK_OPTIONS

  if(_NAME)
  else()
    set(_NAME "$<TARGET_PROPERTY:${TARGET},NAME>")
  endif()

  if(_DESCRIPTION)
  else()
    set(_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}")
  endif()

  if(_VERSION)
  else()
    set(_VERSION "${CPACK_PACKAGE_VERSION}")
  endif()

  if(_URL)
  else()
    set(_URL "${CPACK_PACKAGE_HOMEPAGE_URL}")
  endif()

  if(INSTALL_INCLUDE_DIR)
  else()
    set(INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
  endif()

  if(INSTALL_LIB_DIR)
  else()
    set(INSTALL_LIB_DIR "${CMAKE_INSTALL_LIBDIR}")
  endif()

  set(PKG_CONFIG_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${_NAME}.pc")

  set(PUBLIC_INCLUDES "$<TARGET_PROPERTY:${TARGET},INTERFACE_INCLUDE_DIRECTORIES>")
  set(PUBLIC_LIBS "$<TARGET_PROPERTY:${TARGET},INTERFACE_LINK_LIBRARIES>")
  set(PUBLIC_COMPILE_FLAGS "$<TARGET_PROPERTY:${TARGET},INTERFACE_COMPILE_DEFINITIONS>")

  set("NEEDS_LIBS" "$<NOT:$<STREQUAL:$<TARGET_PROPERTY:${TARGET},TYPE>,INTERFACE_LIBRARY>>")
  string(REPLACE "," "$<COMMA>" NEEDS_LIBS_ESCAPED "${NEEDS_LIBS}")
  string(REPLACE ">" "$<ANGLE-R>" NEEDS_LIBS_ESCAPED "${NEEDS_LIBS_ESCAPED}")

  list(APPEND header "prefix=${CMAKE_INSTALL_PREFIX}")
  list(APPEND header "$<IF:$<OR:$<BOOL:${PUBLIC_LIBS}>,${NEEDS_LIBS}>,libdir=\${prefix}/${INSTALL_LIB_DIR},>")
  list(APPEND header "$<IF:$<BOOL:${PUBLIC_INCLUDES}>,includedir=\${prefix}/${INSTALL_INCLUDE_DIR},>")


  list(APPEND libSpecific "Name: ${_NAME}")
  if(_DESCRIPTION)
    list(APPEND libSpecific "Description: ${_DESCRIPTION}")
  endif()
  if(_URL)
    list(APPEND libSpecific "URL: ${_URL}")
  endif()
  if(_VERSION)
    list(APPEND libSpecific "Version: ${_VERSION}")
  endif()
  if(_REQUIRES)
    list(APPEND libSpecific "Requires: ${_REQUIRES}")
  endif()
  if(_CONFLICTS)
    list(APPEND libSpecific "Conflicts: ${_CONFLICTS}")
  endif()

  set(OTHER_INCLUDE_FLAGS "-I$<JOIN:$<REMOVE_DUPLICATES:${PUBLIC_INCLUDES}>, -I>")  # Not needed, we can only get build interface flags here. Insert them after -I\${includedir} if you find a way to fix/workaround it

  # Here is a workaround to inability to use TARGET_LINKER_FILE_NAME for targets not involving library generation.
  # Strangely $<IF evaluates both branches, not only the one taken, which causes an error
  # We workaround it by generating the subexpression source using $<IF and then evaluating it using $<GENEX_EVAL
  # Of course we could have used conditional expressions on CMake script part, but I have decided to implement it in generator expressions part, so hypthetically all the expressions can be merged into a single file and this function can be made simple

  set(ESCAPED_GENEXPR_BEGINNING "$<1:$><")  # A hack because there is no escape for `$` or `<` or `$<`.  So we just disrupt $< into pieces
  set(CURRENT_LIB_ESCAPED_BINARY_NAME "${ESCAPED_GENEXPR_BEGINNING}TARGET_LINKER_FILE_NAME:${TARGET}$<ANGLE-R>")
  set(LINK_CURRENT_LIB_FLAG "$<GENEX_EVAL:$<IF:${NEEDS_LIBS},-l:${CURRENT_LIB_ESCAPED_BINARY_NAME},>>")

  list(APPEND libSpecific "$<IF:$<OR:$<BOOL:${PUBLIC_LIBS}>,${NEEDS_LIBS}>,Libs: -L\${libdir} ${LINK_CURRENT_LIB_FLAG} $<IF:$<BOOL:${PUBLIC_LIBS}>,-l$<JOIN:$<REMOVE_DUPLICATES:${PUBLIC_LIBS}>, -l>,>,>\n$<IF:$<OR:$<BOOL:${PUBLIC_INCLUDES}>,$<BOOL:${PUBLIC_COMPILE_FLAGS}>>,Cflags: -I\${includedir} $<JOIN:$<REMOVE_DUPLICATES:${PUBLIC_COMPILE_FLAGS}>,>,>")


  list(JOIN header "\n" header)
  list(JOIN libSpecific "\n" libSpecific)
  set(libSpecific "${header}\n\n${libSpecific}")

  #file(WRITE "${PKG_CONFIG_FILE_NAME}" ${libSpecific})
  file(GENERATE OUTPUT "${PKG_CONFIG_FILE_NAME}"
    CONTENT "${libSpecific}"
    #[CONDITION expression]
  )

  install(FILES "${PKG_CONFIG_FILE_NAME}"
    DESTINATION "${_INSTALL_LIB_DIR}/pkgconfig"
    COMPONENT "${_COMPONENT}"
  )
endfunction()
