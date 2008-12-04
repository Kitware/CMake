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


function(add_external_project_download_command name)
  set(added 0)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)


  if(NOT added)
  get_target_property(cvs_repository ${name} AEP_CVS_REPOSITORY)
  if(cvs_repository)
    if(NOT CVS_EXECUTABLE)
      message(FATAL_ERROR "error: could not find cvs for checkout of ${name}")
    endif()

    get_target_property(cvs_module ${name} AEP_CVS_MODULE)
    if(NOT cvs_module)
      message(FATAL_ERROR "error: no CVS_MODULE")
    endif()

    get_target_property(tag ${name} AEP_CVS_TAG)
    set(cvs_tag)
    if(tag)
      set(cvs_tag ${tag})
    endif()

    set(args -d ${cvs_repository} co ${cvs_tag} -d ${name} ${cvs_module})
    set(wd "${source_dir}")

    set(repository ${cvs_repository})
    set(module ${cvs_module})
    set(tag ${cvs_tag})

    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${sentinels_dir}/${name}-cvsinfo.txt"
      @ONLY
    )

    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CVS_EXECUTABLE} ${args}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${wd}
      COMMENT "Performing download step (CVS checkout) for '${name}'"
      DEPENDS "${sentinels_dir}/${name}-cvsinfo.txt"
    )
    set(added 1)
  endif()
  endif(NOT added)


  if(NOT added)
  get_target_property(svn_repository ${name} AEP_SVN_REPOSITORY)
  if(svn_repository)
    if(NOT Subversion_SVN_EXECUTABLE)
      message(FATAL_ERROR "error: could not find svn for checkout of ${name}")
    endif()

    get_target_property(tag ${name} AEP_SVN_TAG)
    set(svn_tag)
    if(tag)
      set(svn_tag ${tag})
    endif()

    set(args co ${svn_repository} ${svn_tag} ${name})
    set(wd "${source_dir}")

    set(repository ${svn_repository})
    set(module)
    set(tag ${svn_tag})

    configure_file(
      "${CMAKE_ROOT}/Modules/RepositoryInfo.txt.in"
      "${sentinels_dir}/${name}-svninfo.txt"
      @ONLY
    )

    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${Subversion_SVN_EXECUTABLE} ${args}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${wd}
      COMMENT "Performing download step (SVN checkout) for '${name}'"
      DEPENDS "${sentinels_dir}/${name}-svninfo.txt"
    )
    set(added 1)
  endif()
  endif(NOT added)


  if(NOT added)
  get_target_property(dir ${name} AEP_DIR)
  if(dir)
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${dir} ${source_dir}/${name}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (DIR copy) for '${name}'"
      DEPENDS ${dir}
    )
    set(added 1)
  endif()
  endif(NOT added)


  if(NOT added)
  get_target_property(tar ${name} AEP_TAR)
  if(tar)
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CMAKE_COMMAND} -Dfilename=${tar} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (TAR untar) for '${name}'"
      DEPENDS ${tar}
    )
    set(added 1)
  endif()
  endif(NOT added)


  if(NOT added)
  get_target_property(tgz ${name} AEP_TGZ)
  if(tgz)
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CMAKE_COMMAND} -Dfilename=${tgz} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (TGZ untar) for '${name}'"
      DEPENDS ${tgz}
    )
    set(added 1)
  endif()
  endif(NOT added)


  if(NOT added)
  get_target_property(tgz_url ${name} AEP_TGZ_URL)
  if(tgz_url)
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CMAKE_COMMAND} -Dremote=${tgz_url} -Dlocal=${downloads_dir}/${name}.tgz -P ${CMAKE_ROOT}/Modules/DownloadFile.cmake
      COMMAND ${CMAKE_COMMAND} -Dfilename=${downloads_dir}/${name} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (TGZ_URL download and untar) for '${name}'"
      DEPENDS ${downloads_dir}/${name}.tgz
    )
    set(added 1)
  endif()
  endif(NOT added)

  if(NOT added)
  get_target_property(tar_url ${name} AEP_TAR_URL)
  if(tar_url)
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-download
      COMMAND ${CMAKE_COMMAND} -Dremote=${tar_url} -Dlocal=${downloads_dir}/${name}.tar -P ${CMAKE_ROOT}/Modules/DownloadFile.cmake
      COMMAND ${CMAKE_COMMAND} -Dfilename=${downloads_dir}/${name} -Dtmp=${tmp_dir}/${name} -Ddirectory=${source_dir}/${name} -P ${CMAKE_ROOT}/Modules/UntarFile.cmake
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-download
      WORKING_DIRECTORY ${source_dir}
      COMMENT "Performing download step (TAR_URL download and untar) for '${name}'"
      DEPENDS ${downloads_dir}/${name}.tar
    )
    set(added 1)
  endif()
  endif(NOT added)


  if(NOT added)
    message(SEND_ERROR "error: no download info for '${name}'")
  endif(NOT added)
endfunction(add_external_project_download_command)


function(add_external_project_configure_command name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)
  get_configure_build_working_dir(${name} working_dir)

  # Create the working_dir for configure, build and install steps:
  #
  add_custom_command(
    OUTPUT ${working_dir}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${working_dir}
    DEPENDS ${sentinels_dir}/${name}-download
    )

  get_target_property(cmd ${name} AEP_CONFIGURE_COMMAND)
  if(cmd STREQUAL "")
    # Explicit empty string means no configure step for this project
    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-configure
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-configure
      WORKING_DIRECTORY ${working_dir}
      COMMENT "No configure step for '${name}'"
      DEPENDS ${working_dir} ${sentinels_dir}/${name}-download
      )
  else()
    if(NOT cmd)
      set(cmd ${CMAKE_COMMAND})
    endif()

    set(args "")
    get_target_property(configure_args ${name} AEP_CONFIGURE_ARGS)
    if(configure_args)
      set(args "${configure_args}")
      separate_arguments(args)
    endif()

    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-configure
      COMMAND ${cmd} ${args}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-configure
      WORKING_DIRECTORY ${working_dir}
      COMMENT "Performing configure step for '${name}'"
      DEPENDS ${working_dir} ${sentinels_dir}/${name}-download
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
      )
  else()
    if(NOT cmd)
      set(cmd ${CMAKE_COMMAND})
    endif()

    get_target_property(args ${name} AEP_BUILD_ARGS)
    if(NOT args)
      set(args --build ${working_dir} --config ${CMAKE_CFG_INTDIR})
    endif()

    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-build
      COMMAND ${cmd} ${args}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-build
      WORKING_DIRECTORY ${working_dir}
      COMMENT "Performing build step for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-configure
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
      )
  else()
    if(NOT cmd)
      set(cmd ${CMAKE_COMMAND})
    endif()

    get_target_property(args ${name} AEP_INSTALL_ARGS)
    if(NOT args)
      set(args --build ${working_dir} --config ${CMAKE_CFG_INTDIR} --target install)
    endif()

    add_custom_command(
      OUTPUT ${sentinels_dir}/${name}-install
      COMMAND ${cmd} ${args}
      COMMAND ${CMAKE_COMMAND} -E touch ${sentinels_dir}/${name}-install
      WORKING_DIRECTORY ${working_dir}
      COMMENT "Performing install step for '${name}'"
      DEPENDS ${sentinels_dir}/${name}-build
      )
  endif()
endfunction(add_external_project_install_command)


function(add_CMakeExternals_target)
  if(NOT TARGET CMakeExternals)
    get_external_project_directories(base_dir build_dir downloads_dir install_dir
      sentinels_dir source_dir tmp_dir)

    add_custom_command(
      OUTPUT ${tmp_dir}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${build_dir}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${downloads_dir}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${install_dir}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${sentinels_dir}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${source_dir}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${tmp_dir}
    )

    add_custom_target(CMakeExternals ALL
      DEPENDS ${tmp_dir}
    )
  endif()
endfunction(add_CMakeExternals_target)


function(add_external_project name)
  get_external_project_directories(base_dir build_dir downloads_dir install_dir
    sentinels_dir source_dir tmp_dir)

  add_CMakeExternals_target()

  add_custom_target(${name} ALL
    DEPENDS ${sentinels_dir}/${name}-install
  )
  set_target_properties(${name} PROPERTIES AEP_IS_EXTERNAL_PROJECT 1)

  # Loop over ARGN by 2's extracting key/value pairs from
  # the non-explicit arguments to the function:
  #
  list(LENGTH ARGN n)
  set(i 0)
  while(i LESS n)
    math(EXPR j ${i}+1)
    list(GET ARGN ${i} key)
    list(GET ARGN ${j} value)
    #message(STATUS "  ${key}='${value}'")

    if(key STREQUAL "BUILD_ARGS")
      set_target_properties(${name} PROPERTIES AEP_BUILD_ARGS "${value}")
    endif()

    if(key STREQUAL "BUILD_COMMAND")
      set_target_properties(${name} PROPERTIES AEP_BUILD_COMMAND "${value}")
    endif()

    if(key STREQUAL "CONFIGURE_ARGS")
      set_target_properties(${name} PROPERTIES AEP_CONFIGURE_ARGS "${value}")
    endif()

    if(key STREQUAL "CONFIGURE_COMMAND")
      set_target_properties(${name} PROPERTIES AEP_CONFIGURE_COMMAND "${value}")
    endif()

    if(key STREQUAL "CONFIGURE_DIR")
      set_target_properties(${name} PROPERTIES AEP_CONFIGURE_DIR "${value}")
    endif()

    if(key STREQUAL "CVS_REPOSITORY")
      set_target_properties(${name} PROPERTIES AEP_CVS_REPOSITORY "${value}")
    endif()

    if(key STREQUAL "CVS_MODULE")
      set_target_properties(${name} PROPERTIES AEP_CVS_MODULE "${value}")
    endif()

    if(key STREQUAL "CVS_TAG")
      set_target_properties(${name} PROPERTIES AEP_CVS_TAG "${value}")
    endif()

    if(key STREQUAL "DEPENDS")
      add_dependencies(${name} ${value})
    endif()

    if(key STREQUAL "DIR")
      set_target_properties(${name} PROPERTIES AEP_DIR "${value}")
    endif()

    if(key STREQUAL "INSTALL_ARGS")
      set_target_properties(${name} PROPERTIES AEP_INSTALL_ARGS "${value}")
    endif()

    if(key STREQUAL "INSTALL_COMMAND")
      set_target_properties(${name} PROPERTIES AEP_INSTALL_COMMAND "${value}")
    endif()

    if(key STREQUAL "SVN_REPOSITORY")
      set_target_properties(${name} PROPERTIES AEP_SVN_REPOSITORY "${value}")
    endif()

    if(key STREQUAL "SVN_TAG")
      set_target_properties(${name} PROPERTIES AEP_SVN_TAG "${value}")
    endif()

    if(key STREQUAL "TAR")
      set_target_properties(${name} PROPERTIES AEP_TAR "${value}")
    endif()

    if(key STREQUAL "TAR_URL")
      set_target_properties(${name} PROPERTIES AEP_TAR_URL "${value}")
    endif()

    if(key STREQUAL "TGZ")
      set_target_properties(${name} PROPERTIES AEP_TGZ "${value}")
    endif()

    if(key STREQUAL "TGZ_URL")
      set_target_properties(${name} PROPERTIES AEP_TGZ_URL "${value}")
    endif()

    math(EXPR i ${i}+2)
  endwhile()

  add_external_project_download_command(${name})
  add_external_project_configure_command(${name})
  add_external_project_build_command(${name})
  add_external_project_install_command(${name})

  add_dependencies(${name} CMakeExternals)
endfunction(add_external_project)
