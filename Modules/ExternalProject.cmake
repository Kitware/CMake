# - Create custom targets to build projects in external trees
# The 'ep_add' function creates a custom target to drive download,
# update/patch, configure, build, and install steps of an external
# project:
#  ep_add(<name>                 # Name for custom target
#    [DEPENDS projects...]       # Targets on which the project depends
#    [PREFIX dir]                # Root dir for entire project
#    [LIST_SEPARATOR sep]        # Sep to be replaced by ; in cmd lines
#    [TMP_DIR dir]               # Directory to store temporary files
#    [STAMP_DIR dir]             # Directory to store step timestamps
#   #--Download step--------------
#    [DOWNLOAD_DIR dir]          # Directory to store downloaded files
#    [DOWNLOAD_COMMAND cmd...]   # Command to download source tree
#    [CVS_REPOSITORY cvsroot]    # CVSROOT of CVS repository
#    [CVS_MODULE mod]            # Module to checkout from CVS repo
#    [CVS_TAG tag]               # Tag to checkout from CVS repo
#    [SVN_REPOSITORY url]        # URL of Subversion repo
#    [SVN_TAG tag]               # Tag to checkout from Subversion repo
#    [URL /.../src.tgz]          # Full path or URL of source
#   #--Update/Patch step----------
#    [UPDATE_COMMAND cmd...]     # Source work-tree update command
#    [PATCH_COMMAND cmd...]      # Command to patch downloaded source
#   #--Configure step-------------
#    [SOURCE_DIR dir]            # Source dir to be used for build
#    [CONFIGURE_COMMAND cmd...]  # Build tree configuration command
#    [CMAKE_COMMAND /.../cmake]  # Specify alternative cmake executable
#    [CMAKE_GENERATOR gen]       # Specify generator for native build
#    [CMAKE_ARGS args...]        # Arguments to CMake command line
#   #--Build step-----------------
#    [BINARY_DIR dir]            # Specify build dir location
#    [BUILD_COMMAND cmd...]      # Command to drive the native build
#    [BUILD_IN_SOURCE 1]         # Use source dir for build dir
#   #--Install step---------------
#    [INSTALL_DIR dir]           # Installation prefix
#    [INSTALL_COMMAND cmd...]    # Command to drive install after build
#    )
# The *_DIR options specify directories for the project, with default
# directories computed as follows.
# If the PREFIX option is given to ep_add() or the EP_PREFIX directory
# property is set, then an external project is built and installed
# under the specified prefix:
#   TMP_DIR      = <prefix>/tmp
#   STAMP_DIR    = <prefix>/src/<name>-stamp
#   DOWNLOAD_DIR = <prefix>/src
#   SOURCE_DIR   = <prefix>/src/<name>
#   BINARY_DIR   = <prefix>/src/<name>-build
#   INSTALL_DIR  = <prefix>
# Otherwise, if the EP_BASE directory property is set then components
# of an external project are stored under the specified base:
#   TMP_DIR      = <base>/tmp/<name>
#   STAMP_DIR    = <base>/Stamp/<name>
#   DOWNLOAD_DIR = <base>/Download/<name>
#   SOURCE_DIR   = <base>/Source/<name>
#   BINARY_DIR   = <base>/Build/<name>
#   INSTALL_DIR  = <base>/Install/<name>
# If no PREFIX, EP_PREFIX, or EP_BASE is specified then the default
# is to set PREIFX to "<name>-prefix".
# Relative paths are interpreted with respect to the build directory
# corresponding to the source directory in which ep_add is invoked.
#
# If SOURCE_DIR is explicitly set to an existing directory the project
# will be built from it.
# Otherwise a download step must be specified using one of the
# DOWNLOAD_COMMAND, CVS_*, SVN_*, or URL options.
# The URL option may refer locally to a directory or source tarball,
# or refer to a remote tarball (e.g. http://.../src.tgz).
#
# The 'ep_add_step' function adds a custom steps to an external project:
#  ep_add_step(<name> <step> # Names of project and custom step
#    [COMMAND cmd...]        # Command line invoked by this step
#    [COMMENT "text..."]     # Text printed when step executes
#    [DEPENDEES steps...]    # Steps on which this step depends
#    [DEPENDERS steps...]    # Steps that depend on this step
#    [DEPENDS files...]      # Files on which this step depends
#    [ALWAYS 1]              # No stamp file, step always runs
#    [WORKING_DIRECTORY dir] # Working directory for command
#    )
# The command line, comment, and working directory of every standard
# and custom step is processed to replace tokens
# <SOURCE_DIR>,
# <BINARY_DIR>,
# <INSTALL_DIR>,
# and <TMP_DIR>
# with corresponding property values.
#
# The 'ep_get' function retrieves external project target properties:
#  ep_get(<name> [prop1 [prop2 [...]]])
# It stores property values in variables of the same name.
# Property names correspond to the keyword argument names of 'ep_add'.

# Pre-compute a regex to match documented keywords for each command.
file(STRINGS "${CMAKE_CURRENT_LIST_FILE}" lines LIMIT_COUNT 100
     REGEX "^#  (  \\[[A-Z_]+ [^]]*\\] +#.*$|[a-z_]+\\()")
foreach(line IN LISTS lines)
  if("${line}" MATCHES "^#  [a-z_]+\\(")
    if(_ep_func)
      set(_ep_keywords_${_ep_func} "${_ep_keywords_${_ep_func}})$")
    endif()
    string(REGEX REPLACE "^#  ([a-z_]+)\\(.*" "\\1" _ep_func "${line}")
    #message("function [${_ep_func}]")
    set(_ep_keywords_${_ep_func} "^(")
    set(_ep_keyword_sep)
  else()
    string(REGEX REPLACE "^#    \\[([A-Z_]+) .*" "\\1" _ep_key "${line}")
    #message("  keyword [${_ep_key}]")
    set(_ep_keywords_${_ep_func}
      "${_ep_keywords_${_ep_func}}${_ep_keyword_sep}${_ep_key}")
    set(_ep_keyword_sep "|")
  endif()
endforeach()
if(_ep_func)
  set(_ep_keywords_${_ep_func} "${_ep_keywords_${_ep_func}})$")
endif()

function(_ep_parse_arguments f name ns args)
  # Transfer the arguments to this function into target properties for the
  # new custom target we just added so that we can set up all the build steps
  # correctly based on target properties.
  #
  # We loop through ARGN and consider the namespace starting with an
  # upper-case letter followed by at least two more upper-case letters
  # or underscores to be keywords.
  set(key)
  foreach(arg IN LISTS args)
    if(arg MATCHES "^[A-Z][A-Z_][A-Z_]+$" AND
        NOT ((arg STREQUAL "${key}") AND (key STREQUAL "COMMAND")) AND
        NOT arg MATCHES "^(TRUE|FALSE)$")
      # Keyword
      set(key "${arg}")
      if(_ep_keywords_${f} AND NOT key MATCHES "${_ep_keywords_${f}}")
        message(AUTHOR_WARNING "unknown ${f} keyword: ${key}")
      endif()
    elseif(key)
      # Value
      if(NOT arg STREQUAL "")
        set_property(TARGET ${name} APPEND PROPERTY ${ns}${key} "${arg}")
      else()
        get_property(have_key TARGET ${name} PROPERTY ${ns}${key} SET)
        if(have_key)
          get_property(value TARGET ${name} PROPERTY ${ns}${key})
          set_property(TARGET ${name} PROPERTY ${ns}${key} "${value};${arg}")
        else()
          set_property(TARGET ${name} PROPERTY ${ns}${key} "${arg}")
        endif()
      endif()
    else()
      # Missing Keyword
      message(AUTHOR_WARNING "value with no keyword in ${f}")
    endif()
  endforeach()
endfunction(_ep_parse_arguments)

define_property(DIRECTORY PROPERTY "EP_BASE" INHERITED
  BRIEF_DOCS "Base directory for External Project storage."
  FULL_DOCS
  "See documentation of the ep_add() function in the "
  "ExternalProject module."
  )

define_property(DIRECTORY PROPERTY "EP_PREFIX" INHERITED
  BRIEF_DOCS "Top prefix for External Project storage."
  FULL_DOCS
  "See documentation of the ep_add() function in the "
  "ExternalProject module."
  )

function(_ep_set_directories name)
  get_property(prefix TARGET ${name} PROPERTY _EP_PREFIX)
  if(NOT prefix)
    get_property(prefix DIRECTORY PROPERTY EP_PREFIX)
    if(NOT prefix)
      get_property(base DIRECTORY PROPERTY EP_BASE)
      if(NOT base)
        set(prefix "${name}-prefix")
      endif()
    endif()
  endif()
  if(prefix)
    set(tmp_default "${prefix}/tmp")
    set(download_default "${prefix}/src")
    set(source_default "${prefix}/src/${name}")
    set(binary_default "${prefix}/src/${name}-build")
    set(stamp_default "${prefix}/src/${name}-stamp")
    set(install_default "${prefix}")
  else() # assert(base)
    set(tmp_default "${base}/tmp/${name}")
    set(download_default "${base}/Download/${name}")
    set(source_default "${base}/Source/${name}")
    set(binary_default "${base}/Build/${name}")
    set(stamp_default "${base}/Stamp/${name}")
    set(install_default "${base}/Install/${name}")
  endif()
  get_property(build_in_source TARGET ${name} PROPERTY _EP_BUILD_IN_SOURCE)
  if(build_in_source)
    get_property(have_binary_dir TARGET ${name} PROPERTY _EP_BINARY_DIR SET)
    if(have_binary_dir)
      message(FATAL_ERROR
        "External project ${name} has both BINARY_DIR and BUILD_IN_SOURCE!")
    endif()
  endif()
  set(top "${CMAKE_CURRENT_BINARY_DIR}")
  set(places stamp download source binary install tmp)
  foreach(var ${places})
    string(TOUPPER "${var}" VAR)
    get_property(${var}_dir TARGET ${name} PROPERTY _EP_${VAR}_DIR)
    if(NOT ${var}_dir)
      set(${var}_dir "${${var}_default}")
    endif()
    if(NOT IS_ABSOLUTE "${${var}_dir}")
      get_filename_component(${var}_dir "${top}/${${var}_dir}" ABSOLUTE)
    endif()
    set_property(TARGET ${name} PROPERTY _EP_${VAR}_DIR "${${var}_dir}")
  endforeach()
  if(build_in_source)
    get_property(source_dir TARGET ${name} PROPERTY _EP_SOURCE_DIR)
    set_property(TARGET ${name} PROPERTY _EP_BINARY_DIR "${source_dir}")
  endif()

  # Make the directories at CMake configure time *and* add a custom command
  # to make them at build time. They need to exist at makefile generation
  # time for Borland make and wmake so that CMake may generate makefiles
  # with "cd C:\short\paths\with\no\spaces" commands in them.
  #
  # Additionally, the add_custom_command is still used in case somebody
  # removes one of the necessary directories and tries to rebuild without
  # re-running cmake.
  foreach(var ${places})
    string(TOUPPER "${var}" VAR)
    get_property(dir TARGET ${name} PROPERTY _EP_${VAR}_DIR)
    file(MAKE_DIRECTORY "${dir}")
    if(NOT EXISTS "${dir}")
      message(FATAL_ERROR "dir '${dir}' does not exist after file(MAKE_DIRECTORY)")
    endif()
  endforeach()
endfunction(_ep_set_directories)

function(ep_get name)
  foreach(var ${ARGN})
    string(TOUPPER "${var}" VAR)
    get_property(${var} TARGET ${name} PROPERTY _EP_${VAR})
    if(NOT ${var})
      message(FATAL_ERROR "External project \"${name}\" has no ${var}")
    endif()
    set(${var} "${${var}}" PARENT_SCOPE)
  endforeach()
endfunction(ep_get)

function(_ep_get_configure_command_id name cfg_cmd_id_var)
  get_target_property(cmd ${name} _EP_CONFIGURE_COMMAND)

  if(cmd STREQUAL "")
    # Explicit empty string means no configure step for this project
    set(${cfg_cmd_id_var} "none" PARENT_SCOPE)
  else()
    if(NOT cmd)
      # Default is "use cmake":
      set(${cfg_cmd_id_var} "cmake" PARENT_SCOPE)
    else()
      # Otherwise we have to analyze the value:
      if(cmd MATCHES "^[^;]*/configure")
        set(${cfg_cmd_id_var} "configure" PARENT_SCOPE)
      elseif(cmd MATCHES "^[^;]*/cmake" AND NOT cmd MATCHES ";-[PE];")
        set(${cfg_cmd_id_var} "cmake" PARENT_SCOPE)
      elseif(cmd MATCHES "config")
        set(${cfg_cmd_id_var} "configure" PARENT_SCOPE)
      else()
        set(${cfg_cmd_id_var} "unknown:${cmd}" PARENT_SCOPE)
      endif()
    endif()
  endif()
endfunction(_ep_get_configure_command_id)

function(_ep_get_build_command name step cmd_var)
  set(cmd "${${cmd_var}}")
  if(NOT cmd)
    set(args)
    _ep_get_configure_command_id(${name} cfg_cmd_id)
    if(cfg_cmd_id STREQUAL "cmake")
      # CMake project.  Select build command based on generator.
      get_target_property(cmake_generator ${name} _EP_CMAKE_GENERATOR)
      if("${cmake_generator}" MATCHES "Make" AND
          "${cmake_generator}" STREQUAL "${CMAKE_GENERATOR}")
        # The project uses the same Makefile generator.  Use recursive make.
        set(cmd "$(MAKE)")
        if(step STREQUAL "INSTALL")
          set(args install)
        endif()
      else()
        # Drive the project with "cmake --build".
        get_target_property(cmake_command ${name} _EP_CMAKE_COMMAND)
        if(cmake_command)
          set(cmd "${cmake_command}")
        else()
          set(cmd "${CMAKE_COMMAND}")
        endif()
        set(args --build ${binary_dir} --config ${CMAKE_CFG_INTDIR})
        if(step STREQUAL "INSTALL")
          list(APPEND args --target install)
        endif()
      endif()
    else() # if(cfg_cmd_id STREQUAL "configure")
      # Non-CMake project.  Guess "make" and "make install".
      set(cmd "make")
      if(step STREQUAL "INSTALL")
        set(args install)
      endif()
    endif()

    # Use user-specified arguments instead of default arguments, if any.
    get_property(have_args TARGET ${name} PROPERTY _EP_${step}_ARGS SET)
    if(have_args)
      get_target_property(args ${name} _EP_${step}_ARGS)
    endif()

    list(APPEND cmd ${args})
  endif()

  set(${cmd_var} "${cmd}" PARENT_SCOPE)
endfunction(_ep_get_build_command)

function(ep_add_step name step)
  set(cmf_dir ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles)
  ep_get(${name} stamp_dir)

  add_custom_command(APPEND
    OUTPUT ${cmf_dir}/${name}-complete
    DEPENDS ${stamp_dir}/${name}-${step}
    )
  _ep_parse_arguments(ep_add_step
                       ${name} _EP_${step}_ "${ARGN}")

  # Steps depending on this step.
  get_property(dependers TARGET ${name} PROPERTY _EP_${step}_DEPENDERS)
  foreach(depender IN LISTS dependers)
    add_custom_command(APPEND
      OUTPUT ${stamp_dir}/${name}-${depender}
      DEPENDS ${stamp_dir}/${name}-${step}
      )
  endforeach()

  # Dependencies on files.
  get_property(depends TARGET ${name} PROPERTY _EP_${step}_DEPENDS)

  # Dependencies on steps.
  get_property(dependees TARGET ${name} PROPERTY _EP_${step}_DEPENDEES)
  foreach(dependee IN LISTS dependees)
    list(APPEND depends ${stamp_dir}/${name}-${dependee})
  endforeach()

  # The command to run.
  get_property(command TARGET ${name} PROPERTY _EP_${step}_COMMAND)
  if(command)
    set(comment "Performing ${step} step for '${name}'")
  else()
    set(comment "No ${step} step for '${name}'")
  endif()
  get_property(work_dir TARGET ${name} PROPERTY _EP_${step}_WORKING_DIRECTORY)

  # Replace list separators.
  get_property(sep TARGET ${name} PROPERTY _EP_LIST_SEPARATOR)
  if(sep AND command)
    string(REPLACE "${sep}" "\\;" command "${command}")
  endif()

  # Replace location tags.
  foreach(var comment command work_dir)
    if(${var})
      foreach(dir SOURCE_DIR BINARY_DIR INSTALL_DIR TMP_DIR)
        get_property(val TARGET ${name} PROPERTY _EP_${dir})
        string(REPLACE "<${dir}>" "${val}" ${var} "${${var}}")
      endforeach()
    endif()
  endforeach()

  # Custom comment?
  get_property(comment_set TARGET ${name} PROPERTY _EP_${step}_COMMENT SET)
  if(comment_set)
    get_property(comment TARGET ${name} PROPERTY _EP_${step}_COMMENT)
  endif()

  # Run every time?
  get_property(always TARGET ${name} PROPERTY _EP_${step}_ALWAYS)
  if(always)
    set_property(SOURCE ${stamp_dir}/${name}-${step} PROPERTY SYMBOLIC 1)
    set(touch)
  else()
    set(touch ${CMAKE_COMMAND} -E touch ${stamp_dir}/${name}-${step})
  endif()

  add_custom_command(
    OUTPUT ${stamp_dir}/${name}-${step}
    COMMENT ${comment}
    COMMAND ${command}
    COMMAND ${touch}
    DEPENDS ${depends}
    WORKING_DIRECTORY ${work_dir}
    VERBATIM
    )
endfunction(ep_add_step)

function(_ep_add_mkdir_command name)
  ep_get(${name}
    source_dir binary_dir install_dir stamp_dir download_dir tmp_dir)
  ep_add_step(${name} mkdir
    COMMENT "Creating directories for '${name}'"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${source_dir}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${binary_dir}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${install_dir}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${tmp_dir}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${stamp_dir}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${download_dir}
    )
endfunction(_ep_add_mkdir_command)

function(_ep_add_download_command name)
  ep_get(${name} source_dir stamp_dir download_dir tmp_dir)

  get_property(cmd_set TARGET ${name} PROPERTY _EP_DOWNLOAD_COMMAND SET)
  get_property(cmd TARGET ${name} PROPERTY _EP_DOWNLOAD_COMMAND)
  get_property(cvs_repository TARGET ${name} PROPERTY _EP_CVS_REPOSITORY)
  get_property(svn_repository TARGET ${name} PROPERTY _EP_SVN_REPOSITORY)
  get_property(url TARGET ${name} PROPERTY _EP_URL)

  # TODO: Perhaps file:// should be copied to download dir before extraction.
  string(REGEX REPLACE "^file://" "" url "${url}")

  set(depends)
  set(comment)
  set(work_dir)

  if(cmd_set)
    set(work_dir ${download_dir})
  elseif(cvs_repository)
    find_package(CVS)
    if(NOT CVS_EXECUTABLE)
      message(FATAL_ERROR "error: could not find cvs for checkout of ${name}")
    endif()

    get_target_property(cvs_module ${name} _EP_CVS_MODULE)
    if(NOT cvs_module)
      message(FATAL_ERROR "error: no CVS_MODULE")
    endif()

    get_property(cvs_tag TARGET ${name} PROPERTY _EP_CVS_TAG)

    set(repository ${cvs_repository})
    set(module ${cvs_module})
    set(tag ${cvs_tag})
    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${stamp_dir}/${name}-cvsinfo.txt"
      @ONLY
      )

    get_filename_component(src_name "${source_dir}" NAME)
    get_filename_component(work_dir "${source_dir}" PATH)
    set(comment "Performing download step (CVS checkout) for '${name}'")
    set(cmd ${CVS_EXECUTABLE} -d ${cvs_repository} -q co ${cvs_tag} -d ${src_name} ${cvs_module})
    list(APPEND depends ${stamp_dir}/${name}-cvsinfo.txt)
  elseif(svn_repository)
    find_package(Subversion)
    if(NOT Subversion_SVN_EXECUTABLE)
      message(FATAL_ERROR "error: could not find svn for checkout of ${name}")
    endif()

    get_property(svn_tag TARGET ${name} PROPERTY _EP_SVN_TAG)

    set(repository ${svn_repository})
    set(module)
    set(tag ${svn_tag})
    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${stamp_dir}/${name}-svninfo.txt"
      @ONLY
      )

    get_filename_component(src_name "${source_dir}" NAME)
    get_filename_component(work_dir "${source_dir}" PATH)
    set(comment "Performing download step (SVN checkout) for '${name}'")
    set(cmd ${Subversion_SVN_EXECUTABLE} co ${svn_repository} ${svn_tag} ${src_name})
    list(APPEND depends ${stamp_dir}/${name}-svninfo.txt)
  elseif(url)
    get_filename_component(work_dir "${source_dir}" PATH)
    set(repository "external project URL")
    set(module "${url}")
    set(tag "")
    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${stamp_dir}/${name}-urlinfo.txt"
      @ONLY
      )
    list(APPEND depends ${stamp_dir}/${name}-urlinfo.txt)
    if(IS_DIRECTORY "${url}")
      get_filename_component(abs_dir "${url}" ABSOLUTE)
      set(comment "Performing download step (DIR copy) for '${name}'")
      set(cmd   ${CMAKE_COMMAND} -E remove_directory ${source_dir}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${abs_dir} ${source_dir})
    else()
      if("${url}" MATCHES "^[a-z]+://")
        # TODO: Should download and extraction be different steps?
        string(REGEX MATCH "\\.(tar|tgz|tar\\.gz)" ext "${url}")
        set(file ${download_dir}/${name}${ext})
        set(cmd ${CMAKE_COMMAND} -Dremote=${url} -Dlocal=${file} -P ${CMAKE_ROOT}/Modules/DownloadFile.cmake
          COMMAND)
        set(comment "Performing download step (download and extract) for '${name}'")
      else()
        set(file "${url}")
        set(comment "Performing download step (extract) for '${name}'")
      endif()
      # TODO: Support other archive formats.
      list(APPEND cmd ${CMAKE_COMMAND} -Dfilename=${file} -Dtmp=${tmp_dir} -Ddirectory=${source_dir} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake)
    endif()
  else()
    message(SEND_ERROR "error: no download info for '${name}'")
  endif()

  ep_add_step(${name} download
    COMMENT ${comment}
    COMMAND ${cmd}
    WORKING_DIRECTORY ${work_dir}
    DEPENDS ${depends}
    DEPENDEES mkdir
    )
endfunction(_ep_add_download_command)


function(_ep_add_update_command name)
  ep_get(${name} source_dir)

  get_property(cmd_set TARGET ${name} PROPERTY _EP_UPDATE_COMMAND SET)
  get_property(cmd TARGET ${name} PROPERTY _EP_UPDATE_COMMAND)
  get_property(cvs_repository TARGET ${name} PROPERTY _EP_CVS_REPOSITORY)
  get_property(svn_repository TARGET ${name} PROPERTY _EP_SVN_REPOSITORY)

  set(work_dir)
  set(comment)
  set(always)

  if(cmd_set)
    set(work_dir ${source_dir})
  elseif(cvs_repository)
    if(NOT CVS_EXECUTABLE)
      message(FATAL_ERROR "error: could not find cvs for update of ${name}")
    endif()
    set(work_dir ${source_dir})
    set(comment "Performing update step (CVS update) for '${name}'")
    get_property(cvs_tag TARGET ${name} PROPERTY _EP_CVS_TAG)
    set(cmd ${CVS_EXECUTABLE} -d ${cvs_repository} -q up -dP ${cvs_tag})
    set(always 1)
  elseif(svn_repository)
    if(NOT Subversion_SVN_EXECUTABLE)
      message(FATAL_ERROR "error: could not find svn for update of ${name}")
    endif()
    set(work_dir ${source_dir})
    set(comment "Performing update step (SVN update) for '${name}'")
    get_property(svn_tag TARGET ${name} PROPERTY _EP_SVN_TAG)
    set(cmd ${Subversion_SVN_EXECUTABLE} up ${svn_tag})
    set(always 1)
  endif()

  ep_add_step(${name} update
    COMMENT ${comment}
    COMMAND ${cmd}
    ALWAYS ${always}
    WORKING_DIRECTORY ${work_dir}
    DEPENDEES download
    )
endfunction(_ep_add_update_command)


function(_ep_add_patch_command name)
  ep_get(${name} source_dir)

  get_property(cmd_set TARGET ${name} PROPERTY _EP_PATCH_COMMAND SET)
  get_property(cmd TARGET ${name} PROPERTY _EP_PATCH_COMMAND)

  set(work_dir)

  if(cmd_set)
    set(work_dir ${source_dir})
  endif()

  ep_add_step(${name} patch
    COMMAND ${cmd}
    WORKING_DIRECTORY ${work_dir}
    DEPENDEES download
    )
endfunction(_ep_add_patch_command)


# TODO: Make sure external projects use the proper compiler
function(_ep_add_configure_command name)
  ep_get(${name} source_dir binary_dir)

  # Depend on other external projects (file-level).
  set(file_deps)
  get_property(deps TARGET ${name} PROPERTY _EP_DEPENDS)
  foreach(dep IN LISTS deps)
    get_property(dep_stamp_dir TARGET ${dep} PROPERTY _EP_STAMP_DIR)
    list(APPEND file_deps ${dep_stamp_dir}/${dep}-done)
  endforeach()

  get_property(cmd_set TARGET ${name} PROPERTY _EP_CONFIGURE_COMMAND SET)
  if(cmd_set)
    get_property(cmd TARGET ${name} PROPERTY _EP_CONFIGURE_COMMAND)
  else()
    get_target_property(cmake_command ${name} _EP_CMAKE_COMMAND)
    if(cmake_command)
      set(cmd "${cmake_command}")
    else()
      set(cmd "${CMAKE_COMMAND}")
    endif()

    get_property(cmake_args TARGET ${name} PROPERTY _EP_CMAKE_ARGS)
    list(APPEND cmd ${cmake_args})

    get_target_property(cmake_generator ${name} _EP_CMAKE_GENERATOR)
    if(cmake_generator)
      list(APPEND cmd "-G${cmake_generator}" "${source_dir}")
    endif()
  endif()

  ep_add_step(${name} configure
    COMMAND ${cmd}
    WORKING_DIRECTORY ${binary_dir}
    DEPENDEES update patch
    DEPENDS ${file_deps}
    )
endfunction(_ep_add_configure_command)


function(_ep_add_build_command name)
  ep_get(${name} binary_dir)

  get_property(cmd_set TARGET ${name} PROPERTY _EP_BUILD_COMMAND SET)
  if(cmd_set)
    get_property(cmd TARGET ${name} PROPERTY _EP_BUILD_COMMAND)
  else()
    _ep_get_build_command(${name} BUILD cmd)
  endif()
  ep_add_step(${name} build
    COMMAND ${cmd}
    WORKING_DIRECTORY ${binary_dir}
    DEPENDEES configure
    )
endfunction(_ep_add_build_command)


function(_ep_add_install_command name)
  ep_get(${name} binary_dir)

  get_property(cmd_set TARGET ${name} PROPERTY _EP_INSTALL_COMMAND SET)
  if(cmd_set)
    get_property(cmd TARGET ${name} PROPERTY _EP_INSTALL_COMMAND)
  else()
    _ep_get_build_command(${name} INSTALL cmd)
  endif()
  ep_add_step(${name} install
    COMMAND ${cmd}
    WORKING_DIRECTORY ${binary_dir}
    DEPENDEES build
    )
endfunction(_ep_add_install_command)

function(ep_add name)
  # Add a custom target for the external project.
  set(cmf_dir ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles)
  add_custom_target(${name} ALL DEPENDS ${cmf_dir}/${name}-complete)
  set_property(TARGET ${name} PROPERTY _EP_IS_EXTERNAL_PROJECT 1)
  _ep_parse_arguments(ep_add ${name} _EP_ "${ARGN}")
  _ep_set_directories(${name})
  ep_get(${name} stamp_dir)

  # The 'complete' step depends on all other steps and creates a
  # 'done' mark.  A dependent external project's 'configure' step
  # depends on the 'done' mark so that it rebuilds when this project
  # rebuilds.  It is important that 'done' is not the output of any
  # custom command so that CMake does not propagate build rules to
  # other external project targets.
  add_custom_command(
    OUTPUT ${cmf_dir}/${name}-complete
    COMMENT "Completed '${name}'"
    COMMAND ${CMAKE_COMMAND} -E touch ${cmf_dir}/${name}-complete
    COMMAND ${CMAKE_COMMAND} -E touch ${stamp_dir}/${name}-done
    DEPENDS ${stamp_dir}/${name}-install
    VERBATIM
    )


  # Depend on other external projects (target-level).
  get_property(deps TARGET ${name} PROPERTY _EP_DEPENDS)
  foreach(arg IN LISTS deps)
    add_dependencies(${name} ${arg})
  endforeach()

  # Set up custom build steps based on the target properties.
  # Each step depends on the previous one.
  #
  # The target depends on the output of the final step.
  # (Already set up above in the DEPENDS of the add_custom_target command.)
  #
  _ep_add_mkdir_command(${name})
  _ep_add_download_command(${name})
  _ep_add_update_command(${name})
  _ep_add_patch_command(${name})
  _ep_add_configure_command(${name})
  _ep_add_build_command(${name})
  _ep_add_install_command(${name})
endfunction(ep_add)
