include(ExternalProject)

# add_subproject(<name> [NO_INSTALL] [DIR <dirname>] [DEPENDS [subpro_dep ...]])
function(add_subproject _name)
  cmake_parse_arguments(_arg "NO_INSTALL" "DIR" "DEPENDS" ${ARGN})

  if(_arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "There are unparsed arguments")
  endif()

  set(_maybe_NO_INSTALL)
  if(_arg_NO_INSTALL)
    set(_maybe_NO_INSTALL INSTALL_COMMAND "")
  endif()

  if(CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
    # Replace list separator before passing on to ExternalProject_Add
    string(REPLACE ";" "|" _CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}")
    string(REPLACE ";" "|" _CROSS_CONFIGS "${CMAKE_CROSS_CONFIGS}")
    string(REPLACE ";" "|" _DEFAULT_CONFIGS "${CMAKE_DEFAULT_CONFIGS}")

    set(_maybe_NINJA_MULTICONFIG_ARGS
      "-DCMAKE_CONFIGURATION_TYPES:STRINGS=${_CONFIGURATION_TYPES}"
      "-DCMAKE_CROSS_CONFIGS:STRINGS=${_CROSS_CONFIGS}"
      "-DCMAKE_DEFAULT_BUILD_TYPE:STRING=${CMAKE_DEFAULT_BUILD_TYPE}"
      "-DCMAKE_DEFAULT_CONFIGS:STRINGS=${_DEFAULT_CONFIGS}"
    )
  endif()

  ExternalProject_Add("${_name}"
    DOWNLOAD_COMMAND      ""
    UPDATE_COMMAND        ""
    UPDATE_DISCONNECTED   ON

    "${_maybe_NO_INSTALL}"

    BUILD_ALWAYS          ON

    LOG_DOWNLOAD          OFF
    LOG_UPDATE            OFF
    LOG_PATCH             OFF
    LOG_CONFIGURE         OFF
    LOG_BUILD             OFF
    LOG_INSTALL           OFF

    SOURCE_DIR            "${PROJECT_SOURCE_DIR}/${_arg_DIR}"

    # Private build directory per subproject
    BINARY_DIR            "${PROJECT_BINARY_DIR}/subproject/${_arg_DIR}"

    # Common install directory, populated immediately
    # during build (during build - not install - of superproject)
    INSTALL_DIR           "${CMAKE_INSTALL_PREFIX}"

    DEPENDS
      ${_arg_DEPENDS}

    LIST_SEPARATOR "|"
    CMAKE_ARGS
      "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>"

      # We can rely on ExternalProject to pick the right
      # generator (and architecture/toolset where applicable),
      # however, we need to explicitly inherit other parent
      # project's build settings.
      "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
      "${_maybe_NINJA_MULTICONFIG_ARGS}"

      # Subproject progress reports clutter up the output, disable
      "-DCMAKE_TARGET_MESSAGES:BOOL=OFF"
      "-DCMAKE_RULE_MESSAGES:BOOL=OFF"
  )
endfunction()
