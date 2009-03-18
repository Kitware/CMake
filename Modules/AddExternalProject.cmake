# Requires CVS CMake for 'function' and '-E touch' and '--build'


find_package(CVS)
find_package(Subversion)


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


function(add_external_project_download_command name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)


  get_target_property(cmd ${name} AEP_DOWNLOAD_COMMAND)
  if(cmd STREQUAL "")
    # Explicit empty string means no download step for this project
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${sentinels_dir}
      COMMENT "No download step for '${name}'"
      DEPENDS ${sentinels_dir}/CMakeExternals-directories
      VERBATIM
      )
    return()
  else()
    if(cmd)
      add_custom_command(
        OUTPUT ${sentinels_dir}/${name}-download
        COMMAND ${cmd}
        COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
        WORKING_DIRECTORY ${downloads_dir}
        COMMENT "Performing download step for '${name}'"
        DEPENDS ${sentinels_dir}/CMakeExternals-directories
        VERBATIM
        )
      return()
    else()
      # No explicit DOWNLOAD_COMMAND property. Look for other properties
      # indicating which download method to use in the logic below...
    endif()
  endif()


  get_target_property(cvs_repository ${name} AEP_CVS_REPOSITORY)
  if(cvs_repository)
    if(NOT CVS_EXECUTABLE)
      message(FATAL_ERROR "error: could not find cvs for checkout of ${name}")
    endif()

    get_target_property(cvs_module ${name} AEP_CVS_MODULE)
    if(NOT cvs_module)
      message(FATAL_ERROR "error: no CVS_MODULE")
    endif()

    get_property(cvs_tag TARGET ${name} PROPERTY AEP_CVS_TAG)

    set(args -d ${cvs_repository} -q co ${cvs_tag} -d ${name} ${cvs_module})

    set(repository ${cvs_repository})
    set(module ${cvs_module})
    set(tag ${cvs_tag})

    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${sentinels_dir}/${name}-cvsinfo.txt"
      @ONLY
    )

    mkdir("${source_dir}/${name}")
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CVS_EXECUTABLE} ${args}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (CVS checkout) for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-cvsinfo.txt
      VERBATIM
    )
    return()
  endif()


  get_target_property(svn_repository ${name} AEP_SVN_REPOSITORY)
  if(svn_repository)
    if(NOT Subversion_SVN_EXECUTABLE)
      message(FATAL_ERROR "error: could not find svn for checkout of ${name}")
    endif()

    get_property(svn_tag TARGET ${name} PROPERTY AEP_SVN_TAG)

    set(args co ${svn_repository} ${svn_tag} ${name})

    set(repository ${svn_repository})
    set(module)
    set(tag ${svn_tag})

    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${sentinels_dir}/${name}-svninfo.txt"
      @ONLY
    )

    mkdir("${source_dir}/${name}")
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${Subversion_SVN_EXECUTABLE} ${args}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (SVN checkout) for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-svninfo.txt
      VERBATIM
    )
    return()
  endif()


  get_target_property(dir ${name} AEP_DIR)
  if(dir)
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
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${source_dir}/${name}
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${abs_dir} ${source_dir}/${name}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (DIR copy) for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-dirinfo.txt
      VERBATIM
    )
    return()
  endif()


  get_target_property(tar ${name} AEP_TAR)
  if(tar)
    mkdir("${source_dir}/${name}")
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CMAKE_COMMAND} -Dfilename=${tar} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (TAR untar) for '${name}'"
      DEPENDS ${tar}
      VERBATIM
    )
    return()
  endif()


  get_target_property(tgz ${name} AEP_TGZ)
  if(tgz)
    mkdir("${source_dir}/${name}")
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CMAKE_COMMAND} -Dfilename=${tgz} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (TGZ untar) for '${name}'"
      DEPENDS ${tgz}
      VERBATIM
    )
    return()
  endif()


  get_target_property(tgz_url ${name} AEP_TGZ_URL)
  if(tgz_url)
    set(repository "add_external_project TGZ_URL")
    set(module "${tgz_url}")
    set(tag "")

    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${sentinels_dir}/${name}-urlinfo.txt"
      @ONLY
    )

    mkdir("${source_dir}/${name}")
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CMAKE_COMMAND} -Dremote=${tgz_url} -Dlocal=${downloads_dir}/${name}.tgz -P ${CMAKE_ROOT}/Modules/DownloadFile.cmake
      COMMAND ${CMAKE_COMMAND} -Dfilename=${downloads_dir}/${name} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (TGZ_URL download and untar) for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-urlinfo.txt
      VERBATIM
    )
    return()
  endif()


  get_target_property(tar_url ${name} AEP_TAR_URL)
  if(tar_url)
    set(repository "add_external_project TAR_URL")
    set(module "${tar_url}")
    set(tag "")

    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${sentinels_dir}/${name}-urlinfo.txt"
      @ONLY
    )

    mkdir("${source_dir}/${name}")
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CMAKE_COMMAND} -Dremote=${tar_url} -Dlocal=${downloads_dir}/${name}.tar -P ${CMAKE_ROOT}/Modules/DownloadFile.cmake
      COMMAND ${CMAKE_COMMAND} -Dfilename=${downloads_dir}/${name} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (TAR_URL download and untar) for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-urlinfo.txt
      VERBATIM
    )
    return()
  endif()


  message(SEND_ERROR "error: no download info for '${name}'")
endfunction(add_external_project_download_command)


function(add_external_project_update_command name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)


  get_target_property(cmd ${name} AEP_UPDATE_COMMAND)
  if(cmd STREQUAL "")
    # Explicit empty string means no update step for this project
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-update
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-update
      WORKING_DIRECTORY ${sentinels_dir}
      COMMENT "No update step for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-download
      )
    return()
  else()
    if(cmd)
      add_custom_command(
        OUTPUT ${sentinels_dir}/${name}-update
        COMMAND ${cmd}
        COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-update
        WORKING_DIRECTORY ${source_dir}/${name}
        COMMENT "Performing update step for '${name}'"
        DEPENDS ${sentinels_dir}/${name}-download
        VERBATIM
        )
      return()
    else()
      # No explicit UPDATE_COMMAND property. Look for other properties
      # indicating which update method to use in the logic below...
    endif()
  endif()


  get_target_property(cvs_repository ${name} AEP_CVS_REPOSITORY)
  if(cvs_repository)
    if(NOT CVS_EXECUTABLE)
      message(FATAL_ERROR "error: could not find cvs for update of ${name}")
    endif()

    get_property(cvs_tag TARGET ${name} PROPERTY AEP_CVS_TAG)

    set(args -d ${cvs_repository} -q up -dP ${cvs_tag})

    mkdir("${source_dir}/${name}")
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-update
      COMMAND ${CVS_EXECUTABLE} ${args}
      WORKING_DIRECTORY ${source_dir}/${name}
      COMMENT "Performing update step (CVS update) for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-download
      VERBATIM
    )
    # Since the update sentinel is not actually written:
    set_property(SOURCE ${sentinels_dir}/${name}-update
      PROPERTY SYMBOLIC 1)
    return()
  endif()


  get_target_property(svn_repository ${name} AEP_SVN_REPOSITORY)
  if(svn_repository)
    if(NOT Subversion_SVN_EXECUTABLE)
      message(FATAL_ERROR "error: could not find svn for update of ${name}")
    endif()

    get_property(svn_tag TARGET ${name} PROPERTY AEP_SVN_TAG)

    set(args up ${svn_tag})

    mkdir("${source_dir}/${name}")
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-update
      COMMAND ${Subversion_SVN_EXECUTABLE} ${args}
      WORKING_DIRECTORY ${source_dir}/${name}
      COMMENT "Performing update step (SVN update) for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-download
      VERBATIM
    )
    # Since the update sentinel is not actually written:
    set_property(SOURCE ${sentinels_dir}/${name}-update
      PROPERTY SYMBOLIC 1)
    return()
  endif()


  add_custom_command(
    OUTPUT ${sentinels_dir}/${name}-update
    COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-update
    WORKING_DIRECTORY ${sentinels_dir}
    COMMENT "No update step for '${name}'"
    DEPENDS ${sentinels_dir}/${name}-download
    VERBATIM
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


function(add_external_project_configure_command name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)
  get_configure_build_working_dir(${name} working_dir)

  get_property(file_deps TARGET ${name} PROPERTY AEP_FILE_DEPENDS)
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
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-complete
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
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-complete
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

function(add_external_project name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)


  # Ensure root CMakeExternals target and directories are created.
  # All external projects will depend on this root CMakeExternals target.
  #
  add_CMakeExternals_target()


  # Add a custom target for the external project and make its DEPENDS
  # the output of the final build step:
  #
  add_custom_target(${name} ALL
    DEPENDS ${sentinels_dir}/${name}-install
  )
  set_target_properties(${name} PROPERTIES AEP_IS_EXTERNAL_PROJECT 1)
  add_dependencies(${name} CMakeExternals)


  # Transfer the arguments to this function into target properties for the
  # new custom target we just added so that we can set up all the build steps
  # correctly based on target properties.
  #
  # We loop through ARGN and consider the namespace starting with an
  # upper-case letter followed by at least two more upper-case letters
  # or underscores to be keywords.
  set(key)
  foreach(arg IN LISTS ARGN)
    if(arg MATCHES "^[A-Z][A-Z_][A-Z_]+$" AND
        NOT arg MATCHES "^(TRUE|FALSE)$")
      # Keyword
      set(key "${arg}")
      if(NOT key MATCHES "${_aep_keyword_regex}")
        message(AUTHOR_WARNING "unknown add_external_project keyword: ${key}")
      endif()
    elseif(key STREQUAL "DEPENDS")
      # Value for DEPENDS
      if(NOT arg STREQUAL "")
        add_dependencies(${name} ${arg})
        set_property(TARGET ${name} APPEND PROPERTY AEP_FILE_DEPENDS "${sentinels_dir}/${arg}-complete")
      else()
        message(AUTHOR_WARNING "empty DEPENDS value in add_external_project")
      endif()
    elseif(key)
      # Value
      if(NOT arg STREQUAL "")
        set_property(TARGET ${name} APPEND PROPERTY AEP_${key} "${arg}")
      else()
        get_property(have_key TARGET ${name} PROPERTY AEP_${key} SET)
        if(have_key)
          get_property(value TARGET ${name} PROPERTY AEP_${key})
          set_property(TARGET ${name} PROPERTY AEP_${key} "${value};${arg}")
        else()
          set_property(TARGET ${name} PROPERTY AEP_${key} "${arg}")
        endif()
      endif()
    else()
      # Missing Keyword
      message(AUTHOR_WARNING "value with no keyword in add_external_project")
    endif()
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
