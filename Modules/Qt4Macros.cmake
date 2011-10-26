# This file is included by FindQt4.cmake, don't include it directly.

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)


######################################
#
#       Macros for building Qt files
#
######################################


macro(QT4_EXTRACT_OPTIONS _qt4_files _qt4_options)
  set(${_qt4_files})
  set(${_qt4_options})
  set(_QT4_DOING_OPTIONS FALSE)
  foreach(_currentArg ${ARGN})
    if("${_currentArg}" STREQUAL "OPTIONS")
      set(_QT4_DOING_OPTIONS TRUE)
    else("${_currentArg}" STREQUAL "OPTIONS")
      if(_QT4_DOING_OPTIONS)
        list(APPEND ${_qt4_options} "${_currentArg}")
      else(_QT4_DOING_OPTIONS)
        list(APPEND ${_qt4_files} "${_currentArg}")
      endif(_QT4_DOING_OPTIONS)
    endif("${_currentArg}" STREQUAL "OPTIONS")
  endforeach(_currentArg)
endmacro(QT4_EXTRACT_OPTIONS)


# macro used to create the names of output files preserving relative dirs
macro(QT4_MAKE_OUTPUT_FILE infile prefix ext outfile )
  string(LENGTH ${CMAKE_CURRENT_BINARY_DIR} _binlength)
  string(LENGTH ${infile} _infileLength)
  set(_checkinfile ${CMAKE_CURRENT_SOURCE_DIR})
  if(_infileLength GREATER _binlength)
    string(SUBSTRING "${infile}" 0 ${_binlength} _checkinfile)
    if(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
      file(RELATIVE_PATH rel ${CMAKE_CURRENT_BINARY_DIR} ${infile})
    else(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
      file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})
    endif(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  else(_infileLength GREATER _binlength)
    file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})
  endif(_infileLength GREATER _binlength)
  if(WIN32 AND rel MATCHES "^[a-zA-Z]:") # absolute path
    string(REGEX REPLACE "^([a-zA-Z]):(.*)$" "\\1_\\2" rel "${rel}")
  endif(WIN32 AND rel MATCHES "^[a-zA-Z]:")
  set(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${rel}")
  string(REPLACE ".." "__" _outfile ${_outfile})
  get_filename_component(outpath ${_outfile} PATH)
  get_filename_component(_outfile ${_outfile} NAME_WE)
  file(MAKE_DIRECTORY ${outpath})
  set(${outfile} ${outpath}/${prefix}${_outfile}.${ext})
endmacro(QT4_MAKE_OUTPUT_FILE )


macro(QT4_GET_MOC_FLAGS _moc_flags)
  set(${_moc_flags})
  get_directory_property(_inc_DIRS INCLUDE_DIRECTORIES)

  foreach(_current ${_inc_DIRS})
    if("${_current}" MATCHES "\\.framework/?$")
      string(REGEX REPLACE "/[^/]+\\.framework" "" framework_path "${_current}")
      set(${_moc_flags} ${${_moc_flags}} "-F${framework_path}")
    else("${_current}" MATCHES "\\.framework/?$")
      set(${_moc_flags} ${${_moc_flags}} "-I${_current}")
    endif("${_current}" MATCHES "\\.framework/?$")
  endforeach(_current ${_inc_DIRS})

  get_directory_property(_defines COMPILE_DEFINITIONS)
  foreach(_current ${_defines})
    set(${_moc_flags} ${${_moc_flags}} "-D${_current}")
  endforeach(_current ${_defines})

  if(Q_WS_WIN)
    set(${_moc_flags} ${${_moc_flags}} -DWIN32)
  endif(Q_WS_WIN)

endmacro(QT4_GET_MOC_FLAGS)


# helper macro to set up a moc rule
macro(QT4_CREATE_MOC_COMMAND infile outfile moc_flags moc_options)
  # For Windows, create a parameters file to work around command line length limit
  if(WIN32)
    # Pass the parameters in a file.  Set the working directory to
    # be that containing the parameters file and reference it by
    # just the file name.  This is necessary because the moc tool on
    # MinGW builds does not seem to handle spaces in the path to the
    # file given with the @ syntax.
    get_filename_component(_moc_outfile_name "${outfile}" NAME)
    get_filename_component(_moc_outfile_dir "${outfile}" PATH)
    if(_moc_outfile_dir)
      set(_moc_working_dir WORKING_DIRECTORY ${_moc_outfile_dir})
    endif(_moc_outfile_dir)
    set(_moc_parameters_file ${outfile}_parameters)
    set(_moc_parameters ${moc_flags} ${moc_options} -o "${outfile}" "${infile}")
    string(REPLACE ";" "\n" _moc_parameters "${_moc_parameters}")
    file(WRITE ${_moc_parameters_file} "${_moc_parameters}")
    add_custom_command(OUTPUT ${outfile}
                       COMMAND ${QT_MOC_EXECUTABLE} @${_moc_outfile_name}_parameters
                       DEPENDS ${infile}
                       ${_moc_working_dir}
                       VERBATIM)
  else(WIN32)
    add_custom_command(OUTPUT ${outfile}
                       COMMAND ${QT_MOC_EXECUTABLE}
                       ARGS ${moc_flags} ${moc_options} -o ${outfile} ${infile}
                       DEPENDS ${infile})
  endif(WIN32)
endmacro(QT4_CREATE_MOC_COMMAND)


macro(QT4_GENERATE_MOC infile outfile )
# get include dirs and flags
   QT4_GET_MOC_FLAGS(moc_flags)
   get_filename_component(abs_infile ${infile} ABSOLUTE)
   set(_outfile "${outfile}")
   if(NOT IS_ABSOLUTE "${outfile}")
     set(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${outfile}")
   endif(NOT IS_ABSOLUTE "${outfile}")
   QT4_CREATE_MOC_COMMAND(${abs_infile} ${_outfile} "${moc_flags}" "")
   set_source_files_properties(${outfile} PROPERTIES SKIP_AUTOMOC TRUE)  # dont run automoc on this file
endmacro(QT4_GENERATE_MOC)


# QT4_WRAP_CPP(outfiles inputfile ... )

macro(QT4_WRAP_CPP outfiles )
  # get include dirs
  QT4_GET_MOC_FLAGS(moc_flags)
  QT4_EXTRACT_OPTIONS(moc_files moc_options ${ARGN})

  foreach(it ${moc_files})
    get_filename_component(it ${it} ABSOLUTE)
    QT4_MAKE_OUTPUT_FILE(${it} moc_ cxx outfile)
    QT4_CREATE_MOC_COMMAND(${it} ${outfile} "${moc_flags}" "${moc_options}")
    set(${outfiles} ${${outfiles}} ${outfile})
  endforeach(it)

endmacro(QT4_WRAP_CPP)


# QT4_WRAP_UI(outfiles inputfile ... )

macro(QT4_WRAP_UI outfiles )
  QT4_EXTRACT_OPTIONS(ui_files ui_options ${ARGN})

  foreach(it ${ui_files})
    get_filename_component(outfile ${it} NAME_WE)
    get_filename_component(infile ${it} ABSOLUTE)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/ui_${outfile}.h)
    add_custom_command(OUTPUT ${outfile}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS ${ui_options} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile})
    set(${outfiles} ${${outfiles}} ${outfile})
  endforeach(it)

endmacro(QT4_WRAP_UI)


# QT4_ADD_RESOURCES(outfiles inputfile ... )

macro(QT4_ADD_RESOURCES outfiles )
  QT4_EXTRACT_OPTIONS(rcc_files rcc_options ${ARGN})

  foreach(it ${rcc_files})
    get_filename_component(outfilename ${it} NAME_WE)
    get_filename_component(infile ${it} ABSOLUTE)
    get_filename_component(rc_path ${infile} PATH)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/qrc_${outfilename}.cxx)
    #  parse file for dependencies
    #  all files are absolute paths or relative to the location of the qrc file
    file(READ "${infile}" _RC_FILE_CONTENTS)
    string(REGEX MATCHALL "<file[^<]+" _RC_FILES "${_RC_FILE_CONTENTS}")
    set(_RC_DEPENDS)
    foreach(_RC_FILE ${_RC_FILES})
      string(REGEX REPLACE "^<file[^>]*>" "" _RC_FILE "${_RC_FILE}")
      if(NOT IS_ABSOLUTE "${_RC_FILE}")
        set(_RC_FILE "${rc_path}/${_RC_FILE}")
      endif(NOT IS_ABSOLUTE "${_RC_FILE}")
      set(_RC_DEPENDS ${_RC_DEPENDS} "${_RC_FILE}")
    endforeach(_RC_FILE)
    # Since this cmake macro is doing the dependency scanning for these files,
    # let's make a configured file and add it as a dependency so cmake is run
    # again when dependencies need to be recomputed.
    QT4_MAKE_OUTPUT_FILE("${infile}" "" "qrc.depends" out_depends)
    configure_file("${infile}" "${out_depends}" COPY_ONLY)
    add_custom_command(OUTPUT ${outfile}
      COMMAND ${QT_RCC_EXECUTABLE}
      ARGS ${rcc_options} -name ${outfilename} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile}
      DEPENDS ${_RC_DEPENDS} "${out_depends}")
    set(${outfiles} ${${outfiles}} ${outfile})
  endforeach(it)

endmacro(QT4_ADD_RESOURCES)


macro(QT4_ADD_DBUS_INTERFACE _sources _interface _basename)
  get_filename_component(_infile ${_interface} ABSOLUTE)
  set(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
  set(_impl   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
  set(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)

  get_source_file_property(_nonamespace ${_interface} NO_NAMESPACE)
  if(_nonamespace)
    set(_params -N -m)
  else(_nonamespace)
    set(_params -m)
  endif(_nonamespace)

  get_source_file_property(_classname ${_interface} CLASSNAME)
  if(_classname)
    set(_params ${_params} -c ${_classname})
  endif(_classname)

  get_source_file_property(_include ${_interface} INCLUDE)
  if(_include)
    set(_params ${_params} -i ${_include})
  endif(_include)

  add_custom_command(OUTPUT ${_impl} ${_header}
      COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} ${_params} -p ${_basename} ${_infile}
      DEPENDS ${_infile})

  set_source_files_properties(${_impl} PROPERTIES SKIP_AUTOMOC TRUE)

  QT4_GENERATE_MOC(${_header} ${_moc})

  set(${_sources} ${${_sources}} ${_impl} ${_header} ${_moc})
  MACRO_ADD_FILE_DEPENDENCIES(${_impl} ${_moc})

endmacro(QT4_ADD_DBUS_INTERFACE)


macro(QT4_ADD_DBUS_INTERFACES _sources)
  foreach(_current_FILE ${ARGN})
    get_filename_component(_infile ${_current_FILE} ABSOLUTE)
    # get the part before the ".xml" suffix
    string(REGEX REPLACE "(.*[/\\.])?([^\\.]+)\\.xml" "\\2" _basename ${_current_FILE})
    string(TOLOWER ${_basename} _basename)
    QT4_ADD_DBUS_INTERFACE(${_sources} ${_infile} ${_basename}interface)
  endforeach(_current_FILE)
endmacro(QT4_ADD_DBUS_INTERFACES)


macro(QT4_GENERATE_DBUS_INTERFACE _header) # _customName OPTIONS -some -options )
  QT4_EXTRACT_OPTIONS(_customName _qt4_dbus_options ${ARGN})

  get_filename_component(_in_file ${_header} ABSOLUTE)
  get_filename_component(_basename ${_header} NAME_WE)

  if(_customName)
    if (IS_ABSOLUTE ${_customName})
      get_filename_component(_containingDir ${_customName} PATH)
      if (NOT EXISTS ${_containingDir})
        file(MAKE_DIRECTORY "${_containingDir}")
      endif()
      set(_target ${_customName})
    else()
      set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_customName})
    endif()
  else(_customName)
    set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.xml)
  endif(_customName)

  add_custom_command(OUTPUT ${_target}
      COMMAND ${QT_DBUSCPP2XML_EXECUTABLE} ${_qt4_dbus_options} ${_in_file} -o ${_target}
      DEPENDS ${_in_file}
  )
endmacro(QT4_GENERATE_DBUS_INTERFACE)


macro(QT4_ADD_DBUS_ADAPTOR _sources _xml_file _include _parentClass) # _optionalBasename _optionalClassName)
  get_filename_component(_infile ${_xml_file} ABSOLUTE)

  set(_optionalBasename "${ARGV4}")
  if(_optionalBasename)
    set(_basename ${_optionalBasename} )
  else(_optionalBasename)
    string(REGEX REPLACE "(.*[/\\.])?([^\\.]+)\\.xml" "\\2adaptor" _basename ${_infile})
    string(TOLOWER ${_basename} _basename)
  endif(_optionalBasename)

  set(_optionalClassName "${ARGV5}")
  set(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
  set(_impl   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
  set(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)

  if(_optionalClassName)
    add_custom_command(OUTPUT ${_impl} ${_header}
       COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} -m -a ${_basename} -c ${_optionalClassName} -i ${_include} -l ${_parentClass} ${_infile}
       DEPENDS ${_infile}
    )
  else(_optionalClassName)
    add_custom_command(OUTPUT ${_impl} ${_header}
       COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} -m -a ${_basename} -i ${_include} -l ${_parentClass} ${_infile}
       DEPENDS ${_infile}
     )
  endif(_optionalClassName)

  QT4_GENERATE_MOC(${_header} ${_moc})
  set_source_files_properties(${_impl} PROPERTIES SKIP_AUTOMOC TRUE)
  MACRO_ADD_FILE_DEPENDENCIES(${_impl} ${_moc})

  set(${_sources} ${${_sources}} ${_impl} ${_header} ${_moc})
endmacro(QT4_ADD_DBUS_ADAPTOR)


macro(QT4_AUTOMOC)
  QT4_GET_MOC_FLAGS(_moc_INCS)

  set(_matching_FILES )
  foreach(_current_FILE ${ARGN})

    get_filename_component(_abs_FILE ${_current_FILE} ABSOLUTE)
    # if "SKIP_AUTOMOC" is set to true, we will not handle this file here.
    # This is required to make uic work correctly:
    # we need to add generated .cpp files to the sources (to compile them),
    # but we cannot let automoc handle them, as the .cpp files don't exist yet when
    # cmake is run for the very first time on them -> however the .cpp files might
    # exist at a later run. at that time we need to skip them, so that we don't add two
    # different rules for the same moc file
    get_source_file_property(_skip ${_abs_FILE} SKIP_AUTOMOC)

    if( NOT _skip AND EXISTS ${_abs_FILE} )

      file(READ ${_abs_FILE} _contents)

      get_filename_component(_abs_PATH ${_abs_FILE} PATH)

      string(REGEX MATCHALL "# *include +[^ ]+\\.moc[\">]" _match "${_contents}")
      if(_match)
        foreach(_current_MOC_INC ${_match})
          string(REGEX MATCH "[^ <\"]+\\.moc" _current_MOC "${_current_MOC_INC}")

          get_filename_component(_basename ${_current_MOC} NAME_WE)
          if(EXISTS ${_abs_PATH}/${_basename}.hpp)
            set(_header ${_abs_PATH}/${_basename}.hpp)
          else(EXISTS ${_abs_PATH}/${_basename}.hpp)
            set(_header ${_abs_PATH}/${_basename}.h)
          endif(EXISTS ${_abs_PATH}/${_basename}.hpp)
          set(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_current_MOC})
          QT4_CREATE_MOC_COMMAND(${_header} ${_moc} "${_moc_INCS}" "")
          MACRO_ADD_FILE_DEPENDENCIES(${_abs_FILE} ${_moc})
        endforeach(_current_MOC_INC)
      endif(_match)
    endif( NOT _skip AND EXISTS ${_abs_FILE} )
  endforeach(_current_FILE)
endmacro(QT4_AUTOMOC)


macro(QT4_CREATE_TRANSLATION _qm_files)
   QT4_EXTRACT_OPTIONS(_lupdate_files _lupdate_options ${ARGN})
   set(_my_sources)
   set(_my_dirs)
   set(_my_tsfiles)
   set(_ts_pro)
   foreach(_file ${_lupdate_files})
     get_filename_component(_ext ${_file} EXT)
     get_filename_component(_abs_FILE ${_file} ABSOLUTE)
     if(_ext MATCHES "ts")
       list(APPEND _my_tsfiles ${_abs_FILE})
     else(_ext MATCHES "ts")
       if(NOT _ext)
         list(APPEND _my_dirs ${_abs_FILE})
       else(NOT _ext)
         list(APPEND _my_sources ${_abs_FILE})
       endif(NOT _ext)
     endif(_ext MATCHES "ts")
   endforeach(_file)
   foreach(_ts_file ${_my_tsfiles})
     if(_my_sources)
       # make a .pro file to call lupdate on, so we don't make our commands too
       # long for some systems
       get_filename_component(_ts_name ${_ts_file} NAME_WE)
       set(_ts_pro ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${_ts_name}_lupdate.pro)
       set(_pro_srcs)
       foreach(_pro_src ${_my_sources})
         set(_pro_srcs "${_pro_srcs} \"${_pro_src}\"")
       endforeach(_pro_src ${_my_sources})
       file(WRITE ${_ts_pro} "SOURCES = ${_pro_srcs}")
     endif(_my_sources)
     add_custom_command(OUTPUT ${_ts_file}
        COMMAND ${QT_LUPDATE_EXECUTABLE}
        ARGS ${_lupdate_options} ${_ts_pro} ${_my_dirs} -ts ${_ts_file}
        DEPENDS ${_my_sources} ${_ts_pro})
   endforeach(_ts_file)
   QT4_ADD_TRANSLATION(${_qm_files} ${_my_tsfiles})
endmacro(QT4_CREATE_TRANSLATION)


macro(QT4_ADD_TRANSLATION _qm_files)
  foreach(_current_FILE ${ARGN})
    get_filename_component(_abs_FILE ${_current_FILE} ABSOLUTE)
    get_filename_component(qm ${_abs_FILE} NAME_WE)
    get_source_file_property(output_location ${_abs_FILE} OUTPUT_LOCATION)
    if(output_location)
      file(MAKE_DIRECTORY "${output_location}")
      set(qm "${output_location}/${qm}.qm")
    else(output_location)
      set(qm "${CMAKE_CURRENT_BINARY_DIR}/${qm}.qm")
    endif(output_location)

    add_custom_command(OUTPUT ${qm}
       COMMAND ${QT_LRELEASE_EXECUTABLE}
       ARGS ${_abs_FILE} -qm ${qm}
       DEPENDS ${_abs_FILE}
    )
    set(${_qm_files} ${${_qm_files}} ${qm})
  endforeach(_current_FILE)
endmacro(QT4_ADD_TRANSLATION)
