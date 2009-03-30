# Requires CVS CMake for 'function' and '-E touch' and '--build'


find_package(CVS)
find_package(Subversion)

function(_aep_parse_arguments f name ns args)
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
      if(_aep_keywords_${f} AND NOT key MATCHES "${_aep_keywords_${f}}")
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
endfunction(_aep_parse_arguments)


function(get_external_project_directories base_dir_var build_dir_var downloads_dir_var install_dir_var sentinels_dir_var source_dir_var tmp_dir_var)
  set(base "${CMAKE_BINARY_DIR}/CMakeExternals")
  set(${base_dir_var} "${base}" PARENT_SCOPE)
  set(${build_dir_var} "${base}/Build" PARENT_SCOPE)
  set(${downloads_dir_var} "${base}/Downloads" PARENT_SCOPE)
  set(${install_dir_var} "${base}/Install" PARENT_SCOPE)
  set(${sentinels_dir_var} "${base}/Sentinels" PARENT_SCOPE)
  set(${source_dir_var} "${base}/Source" PARENT_SCOPE)
  set(${tmp_dir_var} "${base}/tmp" PARENT_SCOPE)
endfunction(get_external_project_directories)


function(get_configure_build_working_dir name working_dir_var)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)

  get_target_property(dir ${name} AEP_CONFIGURE_DIR)
  if(dir)
    if (IS_ABSOLUTE "${dir}")
      set(working_dir "${dir}")
    else()
      set(working_dir "${source_dir}/${name}/${dir}")
    endif()
  else()
    set(working_dir "${build_dir}/${name}")
  endif()

  set(${working_dir_var} "${working_dir}" PARENT_SCOPE)
endfunction(get_configure_build_working_dir)


function(get_configure_command_id name cfg_cmd_id_var)
  get_target_property(cmd ${name} AEP_CONFIGURE_COMMAND)

  if(cmd STREQUAL "")
    # Explicit empty string means no configure step for this project
    set(${cfg_cmd_id_var} "none" PARENT_SCOPE)
  else()
    if(NOT cmd)
      # Default is "use cmake":
      set(${cfg_cmd_id_var} "cmake" PARENT_SCOPE)
    else()
      # Otherwise we have to analyze the value:
      if(cmd MATCHES "/configure$")
        set(${cfg_cmd_id_var} "configure" PARENT_SCOPE)
      else()
        if(cmd MATCHES "cmake")
          set(${cfg_cmd_id_var} "cmake" PARENT_SCOPE)
        else()
          if(cmd MATCHES "config")
            set(${cfg_cmd_id_var} "configure" PARENT_SCOPE)
          else()
            set(${cfg_cmd_id_var} "unknown:${cmd}" PARENT_SCOPE)
          endif()
        endif()
      endif()
    endif()
  endif()
endfunction(get_configure_command_id)

function(_aep_get_build_command name step cmd_var)
  set(cmd "${${cmd_var}}")
  if(NOT cmd)
    set(args)
    get_configure_command_id(${name} cfg_cmd_id)
    if(cfg_cmd_id STREQUAL "cmake")
      # CMake project.  Select build command based on generator.
      get_target_property(cmake_generator ${name} AEP_CMAKE_GENERATOR)
      if("${cmake_generator}" MATCHES "Make" AND
          "${cmake_generator}" STREQUAL "${CMAKE_GENERATOR}")
        # The project uses the same Makefile generator.  Use recursive make.
        set(cmd "$(MAKE)")
        if(step STREQUAL "INSTALL")
          set(args install)
        endif()
      else()
        # Drive the project with "cmake --build".
        get_target_property(cmake_command ${name} AEP_CMAKE_COMMAND)
        if(cmake_command)
          set(cmd "${cmake_command}")
        else()
          set(cmd "${CMAKE_COMMAND}")
        endif()
        set(args --build ${working_dir} --config ${CMAKE_CFG_INTDIR})
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
    get_property(have_args TARGET ${name} PROPERTY AEP_${step}_ARGS SET)
    if(have_args)
      get_target_property(args ${name} AEP_${step}_ARGS)
    endif()

    list(APPEND cmd ${args})
  endif()

  set(${cmd_var} "${cmd}" PARENT_SCOPE)
endfunction(_aep_get_build_command)

function(mkdir d)
  file(MAKE_DIRECTORY "${d}")
  #message(STATUS "mkdir d='${d}'")
  if(NOT EXISTS "${d}")
    message(FATAL_ERROR "error: dir '${d}' does not exist after file(MAKE_DIRECTORY call...")
  endif()
endfunction(mkdir)

# Pre-compute a regex to match known keywords.
set(_aep_keyword_regex "^(")
set(_aep_keyword_sep)
foreach(key IN ITEMS
    COMMAND
    COMMENT
    DEPENDEES
    DEPENDERS
    DEPENDS
    SYMBOLIC
    WORKING_DIRECTORY
    )
  set(_aep_keyword_regex "${_aep_keyword_regex}${_aep_keyword_sep}${key}")
  set(_aep_keyword_sep "|")
endforeach(key)
set(_aep_keyword_regex "${_aep_keyword_regex})$")
set(_aep_keyword_sep)
set(_aep_keywords_add_external_project_step "${_aep_keyword_regex}")

function(add_external_project_step name step)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)
  add_custom_command(APPEND
    OUTPUT ${sentinels_dir}/${name}-complete
    DEPENDS ${sentinels_dir}/${name}-${step}
    )
  _aep_parse_arguments(add_external_project_step
                       ${name} AEP_${step}_ "${ARGN}")

  # Steps depending on this step.
  get_property(dependers TARGET ${name} PROPERTY AEP_${step}_DEPENDERS)
  foreach(depender IN LISTS dependers)
    add_custom_command(APPEND
      OUTPUT ${sentinels_dir}/${name}-${depender}
      DEPENDS ${sentinels_dir}/${name}-${step}
      )
  endforeach()

  # Dependencies on files.
  get_property(depends TARGET ${name} PROPERTY AEP_${step}_DEPENDS)

  # Dependencies on steps.
  get_property(dependees TARGET ${name} PROPERTY AEP_${step}_DEPENDEES)
  foreach(dependee IN LISTS dependees)
    list(APPEND depends ${sentinels_dir}/${name}-${dependee})
  endforeach()

  # The command to run.
  get_property(command TARGET ${name} PROPERTY AEP_${step}_COMMAND)
  if(command)
    set(comment "Performing ${step} step for '${name}'")
  else()
    set(comment "No ${step} step for '${name}'")
  endif()
  get_property(work_dir TARGET ${name} PROPERTY AEP_${step}_WORKING_DIRECTORY)

  # Custom comment?
  get_property(comment_set TARGET ${name} PROPERTY AEP_${step}_COMMENT SET)
  if(comment_set)
    get_property(comment TARGET ${name} PROPERTY AEP_${step}_COMMENT)
  endif()

  # Run every time?
  get_property(symbolic TARGET ${name} PROPERTY AEP_${step}_SYMBOLIC)
  if(symbolic)
    set_property(SOURCE ${sentinels_dir}/${name}-${step} PROPERTY SYMBOLIC 1)
    set(touch)
  else()
    set(touch ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-${step})
  endif()

  add_custom_command(
    OUTPUT ${sentinels_dir}/${name}-${step}
    COMMENT ${comment}
    COMMAND ${command}
    COMMAND ${touch}
    DEPENDS ${depends}
    WORKING_DIRECTORY ${work_dir}
    VERBATIM
    )
endfunction(add_external_project_step)

function(add_external_project_download_command name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)

  get_property(cmd_set TARGET ${name} PROPERTY AEP_DOWNLOAD_COMMAND SET)
  get_property(cmd TARGET ${name} PROPERTY AEP_DOWNLOAD_COMMAND)
  get_property(cvs_repository TARGET ${name} PROPERTY AEP_CVS_REPOSITORY)
  get_property(svn_repository TARGET ${name} PROPERTY AEP_SVN_REPOSITORY)
  get_property(dir TARGET ${name} PROPERTY AEP_DIR)
  get_property(tar TARGET ${name} PROPERTY AEP_TAR)
  get_property(tgz TARGET ${name} PROPERTY AEP_TGZ)
  get_property(tgz_url TARGET ${name} PROPERTY AEP_TGZ_URL)
  get_property(tar_url TARGET ${name} PROPERTY AEP_TAR_URL)

  set(depends ${sentinels_dir}/CMakeExternals-directories)
  set(comment)
  set(work_dir)

  if(cmd_set)
    set(work_dir ${downloads_dir})
  elseif(cvs_repository)
    if(NOT CVS_EXECUTABLE)
      message(FATAL_ERROR "error: could not find cvs for checkout of ${name}")
    endif()

    get_target_property(cvs_module ${name} AEP_CVS_MODULE)
    if(NOT cvs_module)
      message(FATAL_ERROR "error: no CVS_MODULE")
    endif()

    get_property(cvs_tag TARGET ${name} PROPERTY AEP_CVS_TAG)

    set(repository ${cvs_repository})
    set(module ${cvs_module})
    set(tag ${cvs_tag})
    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${sentinels_dir}/${name}-cvsinfo.txt"
      @ONLY
      )

    mkdir("${source_dir}/${name}")
    set(work_dir ${source_dir})
    set(comment "Performing download step (CVS checkout) for '${name}'")
    set(cmd ${CVS_EXECUTABLE} -d ${cvs_repository} -q co ${cvs_tag} -d ${name} ${cvs_module})
    list(APPEND depends ${sentinels_dir}/${name}-cvsinfo.txt)
  elseif(svn_repository)
    if(NOT Subversion_SVN_EXECUTABLE)
      message(FATAL_ERROR "error: could not find svn for checkout of ${name}")
    endif()

    get_property(svn_tag TARGET ${name} PROPERTY AEP_SVN_TAG)

    set(repository ${svn_repository})
    set(module)
    set(tag ${svn_tag})
    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${sentinels_dir}/${name}-svninfo.txt"
      @ONLY
      )

    mkdir("${source_dir}/${name}")
    set(work_dir ${source_dir})
    set(comment "Performing download step (SVN checkout) for '${name}'")
    set(cmd ${Subversion_SVN_EXECUTABLE} co ${svn_repository} ${svn_tag} ${name})
    list(APPEND depends ${sentinels_dir}/${name}-svninfo.txt)
  elseif(dir)
    get_filename_component(abs_dir "${dir}" ABSOLUTE)

    set(repository "add_external_project DIR")
    set(module "${abs_dir}")
    set(tag "")

    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${sentinels_dir}/${name}-dirinfo.txt"
      @ONLY
      )

    mkdir("${source_dir}/${name}")
    set(work_dir ${source_dir})
    set(comment "Performing download step (DIR copy) for '${name}'")
    set(cmd   ${CMAKE_COMMAND} -E remove_directory ${source_dir}/${name}
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${abs_dir} ${source_dir}/${name})
    list(APPEND depends ${sentinels_dir}/${name}-dirinfo.txt)
  elseif(tar)
    mkdir("${source_dir}/${name}")
    set(work_dir ${source_dir})
    set(comment "Performing download step (TAR untar) for '${name}'")
    set(cmd ${CMAKE_COMMAND} -Dfilename=${tar} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake)
    list(APPEND depends ${tar})
  elseif(tgz)
    mkdir("${source_dir}/${name}")
    set(work_dir ${source_dir})
    set(comment "Performing download step (TGZ untar) for '${name}'")
    set(cmd ${CMAKE_COMMAND} -Dfilename=${tgz} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake)
    list(APPEND depends ${tgz})
  elseif(tgz_url)
    set(repository "add_external_project TGZ_URL")
    set(module "${tgz_url}")
    set(tag "")

    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${sentinels_dir}/${name}-urlinfo.txt"
      @ONLY
    )

    mkdir("${source_dir}/${name}")
    set(work_dir ${source_dir})
    set(comment "Performing download step (TGZ_URL download and untar) for '${name}'")
    set(cmd   ${CMAKE_COMMAND} -Dremote=${tgz_url} -Dlocal=${downloads_dir}/${name}.tgz -P ${CMAKE_ROOT}/Modules/DownloadFile.cmake
      COMMAND ${CMAKE_COMMAND} -Dfilename=${downloads_dir}/${name} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake)
    list(APPEND depends ${sentinels_dir}/${name}-urlinfo.txt)
  elseif(tar_url)
    set(repository "add_external_project TAR_URL")
    set(module "${tar_url}")
    set(tag "")

    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${sentinels_dir}/${name}-urlinfo.txt"
      @ONLY
    )

    mkdir("${source_dir}/${name}")
    set(work_dir ${source_dir})
    set(comment "Performing download step (TAR_URL download and untar) for '${name}'")
    set(cmd   ${CMAKE_COMMAND} -Dremote=${tar_url} -Dlocal=${downloads_dir}/${name}.tar -P ${CMAKE_ROOT}/Modules/DownloadFile.cmake
      COMMAND ${CMAKE_COMMAND} -Dfilename=${downloads_dir}/${name} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake)
    list(APPEND depends ${sentinels_dir}/${name}-urlinfo.txt)
  else()
    message(SEND_ERROR "error: no download info for '${name}'")
  endif()

  add_external_project_step(${name} download
    COMMENT ${comment}
    COMMAND ${cmd}
    WORKING_DIRECTORY ${source_dir}
    DEPENDS ${depends}
    )
endfunction(add_external_project_download_command)


function(add_external_project_update_command name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)

  get_property(cmd TARGET ${name} PROPERTY AEP_UPDATE_COMMAND)
  get_property(cvs_repository TARGET ${name} PROPERTY AEP_CVS_REPOSITORY)
  get_property(svn_repository TARGET ${name} PROPERTY AEP_SVN_REPOSITORY)

  set(work_dir)
  set(comment)
  set(symbolic)
  if(cmd)
    set(work_dir ${source_dir}/${name})
  elseif(cvs_repository)
    if(NOT CVS_EXECUTABLE)
      message(FATAL_ERROR "error: could not find cvs for update of ${name}")
    endif()
    mkdir("${source_dir}/${name}")
    set(work_dir ${source_dir}/${name})
    set(comment "Performing update step (CVS update) for '${name}'")
    get_property(cvs_tag TARGET ${name} PROPERTY AEP_CVS_TAG)
    set(cmd ${CVS_EXECUTABLE} -d ${cvs_repository} -q up -dP ${cvs_tag})
    set(symbolic 1)
  elseif(svn_repository)
    if(NOT Subversion_SVN_EXECUTABLE)
      message(FATAL_ERROR "error: could not find svn for update of ${name}")
    endif()
    mkdir("${source_dir}/${name}")
    set(work_dir ${source_dir}/${name})
    set(comment "Performing update step (SVN update) for '${name}'")
    get_property(svn_tag TARGET ${name} PROPERTY AEP_SVN_TAG)
    set(cmd ${Subversion_SVN_EXECUTABLE} up ${svn_tag})
    set(symbolic 1)
  endif()

  add_external_project_step(${name} update
    COMMENT ${comment}
    COMMAND ${cmd}
    SYMBOLIC ${symbolic}
    WORKING_DIRECTORY ${work_dir}
    DEPENDEES download
    )
endfunction(add_external_project_update_command)


function(add_external_project_patch_command name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)

  get_target_property(cmd ${name} AEP_PATCH_COMMAND)
  if(cmd)
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-patch
      COMMAND ${cmd}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-patch
      WORKING_DIRECTORY ${source_dir}/${name}
      COMMENT "Performing patch step for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-download
      VERBATIM
      )
    return()
  endif()

  add_custom_command(
    OUTPUT ${sentinels_dir}/${name}-patch
    COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-patch
    WORKING_DIRECTORY ${sentinels_dir}
    COMMENT "No patch step for '${name}'"
    DEPENDS ${sentinels_dir}/${name}-download
    VERBATIM
    )
endfunction(add_external_project_patch_command)


# TODO: Make sure external projects use the proper compiler
function(add_external_project_configure_command name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)
  get_configure_build_working_dir(${name} working_dir)

  # Depend on other external projects (file-level).
  set(file_deps)
  get_property(deps TARGET ${name} PROPERTY AEP_DEPENDS)
  foreach(arg IN LISTS deps)
    list(APPEND file_deps ${sentinels_dir}/${arg}-done)
  endforeach()
  #message(STATUS "info: name='${name}' file_deps='${file_deps}'")

  # Create the working_dir for configure, build and install steps:
  #
  mkdir("${working_dir}")
  add_custom_command(
    OUTPUT ${sentinels_dir}/${name}-working_dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${working_dir}
    COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-working_dir
    DEPENDS ${sentinels_dir}/${name}-update
            ${sentinels_dir}/${name}-patch
      ${file_deps}
    VERBATIM
    )

  get_target_property(cmd ${name} AEP_CONFIGURE_COMMAND)
  if(cmd STREQUAL "")
    # Explicit empty string means no configure step for this project
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-configure
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-configure
      WORKING_DIRECTORY ${working_dir}
      COMMENT "No configure step for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-working_dir
      VERBATIM
      )
  else()
    if(NOT cmd)
      get_target_property(cmake_command ${name} AEP_CMAKE_COMMAND)
      if(cmake_command)
        set(cmd "${cmake_command}")
      else()
        set(cmd "${CMAKE_COMMAND}")
      endif()

      get_property(cmake_args TARGET ${name} PROPERTY AEP_CMAKE_ARGS)
      list(APPEND cmd ${cmake_args})

      get_target_property(cmake_generator ${name} AEP_CMAKE_GENERATOR)
      if(cmake_generator)
        list(APPEND cmd "-G${cmake_generator}" "${source_dir}/${name}")
      endif()
    endif()

    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-configure
      COMMAND ${cmd}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-configure
      WORKING_DIRECTORY ${working_dir}
      COMMENT "Performing configure step for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-working_dir
      VERBATIM
      )
  endif()
endfunction(add_external_project_configure_command)


function(add_external_project_build_command name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)
  get_configure_build_working_dir(${name} working_dir)

  get_target_property(cmd ${name} AEP_BUILD_COMMAND)
  if(cmd STREQUAL "")
    # Explicit empty string means no build step for this project
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-build
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-build
      WORKING_DIRECTORY ${working_dir}
      COMMENT "No build step for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-configure
      VERBATIM
      )
  else()
    _aep_get_build_command(${name} BUILD cmd)

    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-build
      COMMAND ${cmd}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-build
      WORKING_DIRECTORY ${working_dir}
      COMMENT "Performing build step for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-configure
      VERBATIM
      )
  endif()
endfunction(add_external_project_build_command)


function(add_external_project_install_command name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)
  get_configure_build_working_dir(${name} working_dir)

  get_target_property(cmd ${name} AEP_INSTALL_COMMAND)
  if(cmd STREQUAL "")
    # Explicit empty string means no install step for this project
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-install
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-install
      WORKING_DIRECTORY ${working_dir}
      COMMENT "No install step for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-build
      VERBATIM
      )
  else()
    _aep_get_build_command(${name} INSTALL cmd)

    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-install
      COMMAND ${cmd}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-install
      WORKING_DIRECTORY ${working_dir}
      COMMENT "Performing install step for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-build
      VERBATIM
      )
  endif()
endfunction(add_external_project_install_command)


function(add_CMakeExternals_target)
  if(NOT TARGET CMakeExternals)
    get_external_project_directories(base_dir build_dir downloads_dir install_dir
      sentinels_dir source_dir tmp_dir)

    # Make the directories at CMake configure time *and* add a custom command
    # to make them at build time. They need to exist at makefile generation
    # time for Borland make and wmake so that CMake may generate makefiles
    # with "cd C:\short\paths\with\no\spaces" commands in them.
    #
    # Additionally, the add_custom_command is still used in case somebody
    # removes one of the necessary directories and tries to rebuild without
    # re-running cmake.
    #
    mkdir("${build_dir}")
    mkdir("${downloads_dir}")
    mkdir("${install_dir}")
    mkdir("${sentinels_dir}")
    mkdir("${source_dir}")
    mkdir("${tmp_dir}")

    add_custom_command(
      OUTPUT ${sentinels_dir}/CMakeExternals-directories
      COMMAND ${CMAKE_COMMAND} -E make_directory ${build_dir}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${downloads_dir}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${install_dir}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${sentinels_dir}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${source_dir}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${tmp_dir}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/CMakeExternals-directories
      COMMENT "Creating CMakeExternals directories"
      VERBATIM
    )

    add_custom_target(CMakeExternals ALL
      DEPENDS ${sentinels_dir}/CMakeExternals-directories
    )
  endif()
endfunction(add_CMakeExternals_target)

# Pre-compute a regex to match known keywords.
set(_aep_keyword_regex "^(")
set(_aep_keyword_sep)
foreach(key IN ITEMS
    BUILD_ARGS
    BUILD_COMMAND
    CMAKE_ARGS
    CMAKE_COMMAND
    CMAKE_GENERATOR
    CONFIGURE_COMMAND
    CONFIGURE_DIR
    CVS_MODULE
    CVS_REPOSITORY
    CVS_TAG
    DEPENDS
    DIR
    DOWNLOAD_COMMAND
    INSTALL_ARGS
    INSTALL_COMMAND
    PATCH_COMMAND
    SVN_REPOSITORY
    SVN_TAG
    TAR
    TAR_URL
    TGZ
    TGZ_URL
    UPDATE_COMMAND
    )
  set(_aep_keyword_regex "${_aep_keyword_regex}${_aep_keyword_sep}${key}")
  set(_aep_keyword_sep "|")
endforeach(key)
set(_aep_keyword_regex "${_aep_keyword_regex})$")
set(_aep_keyword_sep)
set(_aep_keywords_add_external_project "${_aep_keyword_regex}")

function(add_external_project name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)


  # Ensure root CMakeExternals target and directories are created.
  # All external projects will depend on this root CMakeExternals target.
  #
  add_CMakeExternals_target()


  # Add a custom target for the external project.  The 'complete' step
  # depends on all other steps and creates a 'done' mark.  A dependent
  # external project's 'configure' step depends on the 'done' mark so
  # that it rebuilds when this project rebuilds.  It is important that
  # 'done' is not the output of any custom command so that CMake does
  # not propagate build rules to other external project targets.
  add_custom_target(${name} ALL DEPENDS ${sentinels_dir}/${name}-complete)
  add_custom_command(
    OUTPUT ${sentinels_dir}/${name}-complete
    COMMENT "Completed '${name}'"
    COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-complete
    COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-done
    DEPENDS ${sentinels_dir}/${name}-install
    VERBATIM
    )
  set_target_properties(${name} PROPERTIES AEP_IS_EXTERNAL_PROJECT 1)
  add_dependencies(${name} CMakeExternals)

  _aep_parse_arguments(add_external_project ${name} AEP_ "${ARGN}")

  # Depend on other external projects (target-level).
  get_property(deps TARGET ${name} PROPERTY AEP_DEPENDS)
  foreach(arg IN LISTS deps)
    add_dependencies(${name} ${arg})
  endforeach()

  # Set up custom build steps based on the target properties.
  # Each step depends on the previous one.
  #
  # The target depends on the output of the final step.
  # (Already set up above in the DEPENDS of the add_custom_target command.)
  #
  add_external_project_download_command(${name})
  add_external_project_update_command(${name})
  add_external_project_patch_command(${name})
  add_external_project_configure_command(${name})
  add_external_project_build_command(${name})
  add_external_project_install_command(${name})
endfunction(add_external_project)
