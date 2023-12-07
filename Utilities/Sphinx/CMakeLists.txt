# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

if(NOT CMake_SOURCE_DIR)
  set(CMakeHelp_STANDALONE 1)
  cmake_minimum_required(VERSION 3.13...3.26 FATAL_ERROR)
  get_filename_component(tmp "${CMAKE_CURRENT_SOURCE_DIR}" PATH)
  get_filename_component(CMake_SOURCE_DIR "${tmp}" PATH)
  include(${CMake_SOURCE_DIR}/Modules/CTestUseLaunchers.cmake)
  include(${CMake_SOURCE_DIR}/Source/CMakeVersion.cmake)
  include(${CMake_SOURCE_DIR}/Source/CMakeInstallDestinations.cmake)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake @ONLY)
  unset(CMAKE_DATA_DIR)
  unset(CMAKE_DATA_DIR CACHE)
  macro(CMake_OPTIONAL_COMPONENT)
    set(COMPONENT "")
  endmacro()
endif()
project(CMakeHelp NONE)

option(SPHINX_INFO "Build Info manual with Sphinx" OFF)
option(SPHINX_MAN "Build man pages with Sphinx" OFF)
option(SPHINX_HTML "Build html help with Sphinx" OFF)
option(SPHINX_SINGLEHTML "Build html single page help with Sphinx" OFF)
option(SPHINX_LINKCHECK "Check external links mentioned in documentation" OFF)
option(SPHINX_QTHELP "Build Qt help with Sphinx" OFF)
option(SPHINX_LATEXPDF "Build PDF help with Sphinx using LaTeX" OFF)
option(SPHINX_TEXT "Build text help with Sphinx (not installed)" OFF)
find_program(SPHINX_EXECUTABLE
  NAMES sphinx-build
  DOC "Sphinx Documentation Builder (sphinx-doc.org)"
  )
set(SPHINX_FLAGS "" CACHE STRING "Flags to pass to sphinx-build")
separate_arguments(sphinx_flags UNIX_COMMAND "${SPHINX_FLAGS}")

mark_as_advanced(SPHINX_TEXT)
mark_as_advanced(SPHINX_FLAGS)

if(NOT (SPHINX_INFO
  OR SPHINX_MAN
  OR SPHINX_HTML
  OR SPHINX_SINGLEHTML
  OR SPHINX_LINKCHECK
  OR SPHINX_QTHELP
  OR SPHINX_TEXT
  OR SPHINX_LATEXPDF
 ))
  return()
elseif(NOT SPHINX_EXECUTABLE)
  message(FATAL_ERROR "SPHINX_EXECUTABLE (sphinx-build) is not found!")
endif()

set(copyright_line_regex "^Copyright (2000-20[0-9][0-9] Kitware.*)")
file(STRINGS "${CMake_SOURCE_DIR}/Copyright.txt" copyright_line
  LIMIT_COUNT 1 REGEX "${copyright_line_regex}")
if(copyright_line MATCHES "${copyright_line_regex}")
  set(conf_copyright "${CMAKE_MATCH_1}")
else()
  set(conf_copyright "Kitware, Inc.")
endif()

if(CMake_SPHINX_CMAKE_ORG)
  set(conf_baseurl "https://cmake.org/cmake/help/latest")
else()
  set(conf_baseurl "")
endif()

set(conf_docs "${CMake_SOURCE_DIR}/Help")
set(conf_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(conf_version "${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}.${CMake_VERSION_PATCH}")
set(conf_release "${CMake_VERSION}")
configure_file(conf.py.in conf.py @ONLY)

set(doc_formats "")
if(SPHINX_HTML)
  list(APPEND doc_formats html)

  # we provide the path to the produced html output in the console
  # for tools that support URI protocol schemes
  set(html_post_commands
    COMMAND ${CMAKE_COMMAND} -E echo "sphinx-build html: HTML documentation generated in file://${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
  )

endif()
if(SPHINX_MAN)
  list(APPEND doc_formats man)
endif()
if(SPHINX_SINGLEHTML)
  list(APPEND doc_formats singlehtml)
endif()
if(SPHINX_LINKCHECK)
  list(APPEND doc_formats linkcheck)
  #
  set(linkcheck_post_commands
    COMMAND ${CMAKE_COMMAND} -E echo "sphinx-build linkcheck: see checking status in file://${CMAKE_CURRENT_BINARY_DIR}/linkcheck/output.txt"
  )
endif()
if(SPHINX_TEXT)
  list(APPEND doc_formats text)
endif()
if(SPHINX_INFO)
  find_program(MAKEINFO_EXECUTABLE
    NAMES makeinfo
    DOC "makeinfo tool"
    )
  if (NOT MAKEINFO_EXECUTABLE)
    message(FATAL_ERROR "MAKEINFO_EXECUTABLE (makeinfo) not found!")
  endif()
  list(APPEND doc_formats texinfo)

  # Sphinx texinfo builder supports .info, .txt, .html and .pdf output.
  # SPHINX_INFO controls the .info output.
  set(texinfo_post_commands
    COMMAND ${MAKEINFO_EXECUTABLE} --no-split -o
      ${CMAKE_CURRENT_BINARY_DIR}/texinfo/cmake.info
      ${CMAKE_CURRENT_BINARY_DIR}/texinfo/cmake.texi
  )
endif()
if(SPHINX_QTHELP)
  find_package(Python REQUIRED)

  find_program(QHELPGENERATOR_EXECUTABLE
    NAMES qhelpgenerator-qt5 qhelpgenerator
    DOC "qhelpgenerator tool"
    )
  if(NOT QHELPGENERATOR_EXECUTABLE)
    message(FATAL_ERROR "QHELPGENERATOR_EXECUTABLE (qhelpgenerator) not found!")
  endif()
  list(APPEND doc_formats qthelp)

  set(qthelp_post_commands
    # Workaround for assistant prior to
    # https://codereview.qt-project.org/#change,82250 in Qt 4.
    COMMAND ${CMAKE_COMMAND} "-DCSS_DIR=${CMAKE_CURRENT_BINARY_DIR}/qthelp/_static"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/apply_qthelp_css_workaround.cmake"
    # Workaround sphinx configurability:
    # https://github.com/sphinx-doc/sphinx/issues/1448
    COMMAND ${CMAKE_COMMAND} "-DQTHELP_DIR=${CMAKE_CURRENT_BINARY_DIR}/qthelp/"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/fixup_qthelp_names.cmake"

    # Create proper identifiers. Workaround for
    # https://github.com/sphinx-doc/sphinx/issues/1491
    COMMAND "${Python_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/create_identifiers.py"
      "${CMAKE_CURRENT_BINARY_DIR}/qthelp/"

    COMMAND ${QHELPGENERATOR_EXECUTABLE}
      ${CMAKE_CURRENT_BINARY_DIR}/qthelp/CMake.qhcp
  )
endif()
if(SPHINX_LATEXPDF)
  list(APPEND doc_formats latexpdf)
endif()

set(doc_html_opts "")
if(CMake_SPHINX_CMAKE_ORG)
  list(APPEND doc_html_opts
    -A googleanalytics=1
    -A opensearch=1
    -A versionswitch=1
    )

  if(CMake_SPHINX_CMAKE_ORG_OUTDATED)
    list(APPEND doc_html_opts -A outdated=1)
  endif()

  list(APPEND html_pre_commands
    COMMAND ${CMAKE_COMMAND} -Dversion=${CMake_VERSION} -P ${CMAKE_CURRENT_SOURCE_DIR}/tutorial_archive.cmake
    )

  list(APPEND qthelp_post_commands
    COMMAND ${CMAKE_COMMAND} -E copy
      "${CMAKE_CURRENT_BINARY_DIR}/qthelp/CMake.qch"
      "${CMAKE_CURRENT_BINARY_DIR}/html/CMake.qch"
    )
endif()

# Redirect `sphinx-build` output to `build-<format>.log` file?
set(sphinx_use_build_log TRUE)
set(sphinx_verbose_levels "DEBUG;TRACE")
set(sphinx_no_redirect_levels "VERBOSE;${sphinx_verbose_levels}")
# NOTE There is no generic verbosity level for all supported generators,
# so lets use CMake verbosity level to control if `sphinx-build` should
# redirect it's output to a file or a user wants to see it at build time.
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.25)
  cmake_language(GET_MESSAGE_LOG_LEVEL verbose_level)
else()
  # If building under CMake < 3.25, fallback to `CMAKE_MESSAGE_LOG_LEVEL`
  # variable.  It was added in 3.17 but it's OK to set it even for older
  # versions (w/o any effect on `message()` command of course).
  set(verbose_level ${CMAKE_MESSAGE_LOG_LEVEL})
endif()
if(DEFINED ENV{VERBOSE} OR CMAKE_VERBOSE_MAKEFILE OR verbose_level IN_LIST sphinx_no_redirect_levels)
  set(sphinx_use_build_log FALSE)
  if(verbose_level IN_LIST sphinx_verbose_levels)
    # NOTE Sphinx accept multiple `-v` options for more verbosity
    # but the output mostly for Sphinx developers...
    list(APPEND sphinx_flags "-v")
  endif()
endif()

set(doc_format_outputs "")
set(doc_format_last "")
foreach(format IN LISTS doc_formats)
  set(doc_format_output "doc_format_${format}")
  set(doc_format_log "")
  set(build_comment_tail " ...")
  if(sphinx_use_build_log)
    set(doc_format_log "build-${format}.log")
    set(build_comment_tail ": see Utilities/Sphinx/${doc_format_log}")
    list(PREPEND doc_format_log ">")
  endif()
  if(CMake_SPHINX_CMAKE_ORG)
    set(doctrees "doctrees/${format}")
  else()
    set(doctrees "doctrees")
  endif()
  if(format STREQUAL "latexpdf")
    # This format does not use builder (-b) but make_mode (-M) which expects
    # arguments in peculiar order
    set(_args
        -M ${format}
        ${CMake_SOURCE_DIR}/Help
        ${CMAKE_CURRENT_BINARY_DIR}/${format}
        -c ${CMAKE_CURRENT_BINARY_DIR}
        -d ${CMAKE_CURRENT_BINARY_DIR}/${doctrees}
        ${sphinx_flags}
        ${doc_${format}_opts}
      )
  else()
    # other formats use standard builder (-b) mode
    set(_args
        -c ${CMAKE_CURRENT_BINARY_DIR}
        -d ${CMAKE_CURRENT_BINARY_DIR}/${doctrees}
        -b ${format}
        ${sphinx_flags}
        ${doc_${format}_opts}
        ${CMake_SOURCE_DIR}/Help
        ${CMAKE_CURRENT_BINARY_DIR}/${format}
      )
  endif()

  add_custom_command(
    OUTPUT ${doc_format_output}
    ${${format}_pre_commands}
    COMMAND ${SPHINX_EXECUTABLE} ${_args} ${doc_format_log}
    ${${format}_post_commands}
    DEPENDS ${doc_format_last}
    COMMENT "sphinx-build ${format}${build_comment_tail}"
    VERBATIM
    )
  set_property(SOURCE ${doc_format_output} PROPERTY SYMBOLIC 1)
  list(APPEND doc_format_outputs ${doc_format_output})
  if(NOT CMake_SPHINX_CMAKE_ORG)
    set(doc_format_last ${doc_format_output})
  endif()
endforeach()

add_custom_target(documentation ALL DEPENDS ${doc_format_outputs})

if(CMake_SPHINX_DEPEND_ON_EXECUTABLES)
  foreach(t IN ITEMS cmake ccmake cmake-gui cpack ctest)
    if(TARGET ${t})
      # Build documentation after main executables.
      add_dependencies(documentation ${t})
    endif()
  endforeach()
endif()

if(CMake_SPHINX_CMAKE_ORG)
  return()
endif()

if(SPHINX_INFO)
  CMake_OPTIONAL_COMPONENT(sphinx-info)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/texinfo/cmake.info
          DESTINATION ${CMAKE_INFO_DIR}
          ${COMPONENT}
          )
endif()

if(SPHINX_MAN)
  file(GLOB man_rst RELATIVE ${CMake_SOURCE_DIR}/Help/manual
    ${CMake_SOURCE_DIR}/Help/manual/*.[1-9].rst)
  foreach(m IN LISTS man_rst)
    if("x${m}" MATCHES "^x(.+)\\.([1-9])\\.rst$")
      set(name "${CMAKE_MATCH_1}")
      set(sec "${CMAKE_MATCH_2}")
      set(skip FALSE)
      if(NOT CMakeHelp_STANDALONE)
        if(name STREQUAL "ccmake" AND NOT BUILD_CursesDialog)
          set(skip TRUE)
        elseif(name STREQUAL "cmake-gui" AND NOT BUILD_QtDialog)
          set(skip TRUE)
        endif()
      endif()
      if(NOT skip)
        CMake_OPTIONAL_COMPONENT(sphinx-man)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/${name}.${sec}
                DESTINATION ${CMAKE_MAN_DIR}/man${sec}
                ${COMPONENT})
      endif()
      unset(skip)
    endif()
  endforeach()
endif()

if(SPHINX_HTML)
  CMake_OPTIONAL_COMPONENT(sphinx-html)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
          DESTINATION ${CMAKE_DOC_DIR}
          ${COMPONENT}
          PATTERN .buildinfo EXCLUDE
          )
endif()

if(SPHINX_SINGLEHTML)
  CMake_OPTIONAL_COMPONENT(sphinx-singlehtml)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/singlehtml
          DESTINATION ${CMAKE_DOC_DIR}
          ${COMPONENT}
          PATTERN .buildinfo EXCLUDE
          )
endif()

if(SPHINX_QTHELP)
  CMake_OPTIONAL_COMPONENT(sphinx-qthelp)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qthelp/CMake.qch
          DESTINATION ${CMAKE_DOC_DIR} ${COMPONENT}
          )
endif()

if(SPHINX_LATEXPDF)
  CMake_OPTIONAL_COMPONENT(sphinx-latexpdf)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/latexpdf/latex/CMake.pdf
          DESTINATION ${CMAKE_DOC_DIR} ${COMPONENT}
          )
endif()
