#
# CMakeIngestOSXBundleLibraries.cmake
#
# Only for the Mac build.
#
# Depends on OS tools:
#   otool
#   install_name_tool
#
# This script ingests libraries and frameworks into an existing .app bundle and
# then uses install_name_tool to fixup the references to the newly embedded
# libraries so that they all refer to each other via "@executable_path."
#
# The main intent (and simplifying assumption used for developing the script)
# is to have a single executable .app bundle that becomes "self-contained" by
# copying all non-system libs that it depends on into itself. The further
# assumption is that all such dependencies are simple .dylib shared library
# files or Mac Framework libraries.
#
# This script can be used as part of the build via ADD_CUSTOM_COMMAND, or used
# only during make install via INSTALL SCRIPT.
#
IF(NOT DEFINED input_file)
  MESSAGE(FATAL_ERROR "
${CMAKE_CURRENT_LIST_FILE}(${CMAKE_CURRENT_LIST_LINE}): error: Variable input_file is not defined.

Use a command line like this to use this script:
  cmake \"-Dinput_file=filename\" \"-Dextra_libs=/path/to/lib1;/path/to/lib2\" \"-Dlib_path=/path/to/unqualified/libs\" -P \"${CMAKE_CURRENT_LIST_FILE}\"

'input_file' should be the main executable inside a Mac bundle directory structure.
For example, use 'bin/paraview.app/Contents/MacOS/paraview' from a ParaView binary dir.

'extra_libs' should be a semi-colon separated list of full path names to extra libraries
to copy into the bundle that cannot be derived from otool -L output. For example, you may
also want to fixup dynamically loaded plugins from your build tree and copy them into the
bundle.

'lib_path' should be the path where to find libraries referenced without a path name in
otool -L output.

")
ENDIF(NOT DEFINED input_file)
message("ingest ${input_file}")
SET(eol_char "E")

IF(APPLE)
  SET(dep_tool "otool")
  SET(dep_cmd_args "-L")
  SET(dep_regex "^\t([^\t]+) \\(compatibility version ([0-9]+.[0-9]+.[0-9]+), current version ([0-9]+.[0-9]+.[0-9]+)\\)${eol_char}$")
ENDIF(APPLE)

MESSAGE("")
MESSAGE("# Script \"${CMAKE_CURRENT_LIST_FILE}\" running...")
MESSAGE("")
MESSAGE("input_file: '${input_file}'")
MESSAGE("extra_libs: '${extra_libs}'")
MESSAGE("lib_path: '${lib_path}'")
MESSAGE("")

GET_FILENAME_COMPONENT(input_file_full "${input_file}" ABSOLUTE)
MESSAGE("input_file_full: '${input_file_full}'")

GET_FILENAME_COMPONENT(bundle "${input_file_full}/../../.." ABSOLUTE)
MESSAGE("bundle: '${bundle}'")


FIND_PROGRAM(dep_cmd ${dep_tool})


MACRO(APPEND_UNIQUE au_list_var au_value)
  SET(${au_list_var} ${${au_list_var}} "${au_value}")
ENDMACRO(APPEND_UNIQUE)


MACRO(GATHER_DEPENDENTS gd_target gd_dependents_var)
  EXECUTE_PROCESS(
    COMMAND ${dep_cmd} ${dep_cmd_args} ${gd_target}
    OUTPUT_VARIABLE dep_tool_ov
    )

  STRING(REGEX REPLACE ";" "\\\\;" dep_candidates "${dep_tool_ov}")
  STRING(REGEX REPLACE "\n" "${eol_char};" dep_candidates "${dep_candidates}")

  SET(${gd_dependents_var} "")

  FOREACH(candidate ${dep_candidates})
  IF("${candidate}" MATCHES "${dep_regex}")
    STRING(REGEX REPLACE "${dep_regex}" "\\1" raw_item "${candidate}")
    STRING(REGEX REPLACE "${dep_regex}" "\\2" raw_compat_version "${candidate}")
    STRING(REGEX REPLACE "${dep_regex}" "\\3" raw_current_version "${candidate}")

    SET(item "${raw_item}")

    STRING(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\1" compat_major_version "${raw_compat_version}")
    STRING(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\2" compat_minor_version "${raw_compat_version}")
    STRING(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" compat_patch_version "${raw_compat_version}")

    STRING(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\1" current_major_version "${raw_current_version}")
    STRING(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\2" current_minor_version "${raw_current_version}")
    STRING(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" current_patch_version "${raw_current_version}")

    #MESSAGE("${raw_item} - compat ${raw_compat_version} - current ${raw_current_version}")
    APPEND_UNIQUE("${gd_dependents_var}" "${item}")
  ELSE("${candidate}" MATCHES "${dep_regex}")
    IF("${candidate}" STREQUAL "${gd_target}:${eol_char}")
      #MESSAGE("info: ignoring target name...")
    ELSE("${candidate}" STREQUAL "${gd_target}:${eol_char}")
      MESSAGE("error: candidate='${candidate}'")
    ENDIF("${candidate}" STREQUAL "${gd_target}:${eol_char}")
  ENDIF("${candidate}" MATCHES "${dep_regex}")
  ENDFOREACH(candidate)
ENDMACRO(GATHER_DEPENDENTS)


MESSAGE("Gathering dependent libraries for '${input_file_full}'...")
GATHER_DEPENDENTS("${input_file_full}" deps)
MESSAGE("")


# Order lexicographically:
#
LIST(SORT deps)


# Split into separate lists, "system" "embedded" and "nonsystem" libraries.
# System libs are assumed to be available on all target runtime Macs and do not
# need to be copied/fixed-up by this script. Embedded libraries are assumed to
# be in the bundle and fixed-up already. Only non-system, non-embedded libs
# need copying and fixing up...
#
SET(system_deps "")
SET(embedded_deps "")
SET(nonsystem_deps "")

FOREACH(d ${deps})
  SET(d_is_embedded_lib 0)
  SET(d_is_system_lib 0)

  IF("${d}" MATCHES "^(/System/Library|/usr/lib)")
    SET(d_is_system_lib 1)
  ELSE("${d}" MATCHES "^(/System/Library|/usr/lib)")
    IF("${d}" MATCHES "^@executable_path")
      SET(d_is_embedded_lib 1)
    ENDIF("${d}" MATCHES "^@executable_path")
  ENDIF("${d}" MATCHES "^(/System/Library|/usr/lib)")

  IF(d_is_system_lib)
    SET(system_deps ${system_deps} "${d}")
  ELSE(d_is_system_lib)
    IF(d_is_embedded_lib)
      SET(embedded_deps ${embedded_deps} "${d}")
    ELSE(d_is_embedded_lib)
      # if the non system lib is not found then try to look for it
      # in the standard framework search path for OSX
      IF(NOT EXISTS "${d}")
        SET(FRAMEWORK_SEARCH "/Library/Frameworks"
          "/System/Library/Frameworks" )
        SET(__FOUND )
        FOREACH(f ${FRAMEWORK_SEARCH})
          SET(newd "${f}/${d}")
          IF(EXISTS "${newd}" AND NOT __FOUND)
            SET(d "${newd}")
            SET(__FOUND TRUE)
          ENDIF(EXISTS "${newd}" AND NOT __FOUND)
        ENDFOREACH(f)
      ENDIF(NOT EXISTS "${d}")
      SET(nonsystem_deps ${nonsystem_deps} "${d}")
    ENDIF(d_is_embedded_lib)
  ENDIF(d_is_system_lib)
ENDFOREACH(d)

MESSAGE("")
MESSAGE("system_deps:")
FOREACH(d ${system_deps})
  MESSAGE("${d}")
ENDFOREACH(d ${system_deps})

MESSAGE("")
MESSAGE("embedded_deps:")
FOREACH(d ${embedded_deps})
  MESSAGE("${d}")
ENDFOREACH(d ${embedded_deps})

MESSAGE("")
MESSAGE("nonsystem_deps:")
FOREACH(d ${nonsystem_deps})
  MESSAGE("${d}")
ENDFOREACH(d ${nonsystem_deps})

MESSAGE("")


MACRO(COPY_LIBRARY_INTO_BUNDLE clib_bundle clib_libsrc clib_dstlibs clib_fixups)
  #
  # If the source library is a framework, copy just the shared lib bit of the framework
  # into the bundle under "${clib_bundle}/Contents/Frameworks" - if it is just a dylib
  # copy it into the same directory with the main bundle executable under
  # "${clib_bundle}/Contents/MacOS"
  #
  IF("${clib_libsrc}" MATCHES ".framework/.*/.*/.*")
    GET_FILENAME_COMPONENT(fw_src "${clib_libsrc}" ABSOLUTE)
    GET_FILENAME_COMPONENT(fw_srcdir "${clib_libsrc}/../../.." ABSOLUTE)
    GET_FILENAME_COMPONENT(fwdirname "${fw_srcdir}" NAME)
    STRING(REGEX REPLACE "^(.*)\\.framework$" "\\1" fwname "${fwdirname}")
    STRING(REGEX REPLACE "^.*/${fwname}\\.framework/(.*)$" "\\1" fwlibname "${clib_libsrc}")
    SET(fw_dstdir "${clib_bundle}/Contents/Frameworks/${fwdirname}")

#    MESSAGE("")
#    MESSAGE("fwdirname: '${fwdirname}'")
#    MESSAGE("fwname: '${fwname}'")
#    MESSAGE("fwlibname: '${fwlibname}'")
#    MESSAGE("fw_src: '${fw_src}'")
#    MESSAGE("fw_srcdir: '${fw_srcdir}'")
#    MESSAGE("fw_dstdir: '${fw_dstdir}'")
#    MESSAGE("new_name: '@executable_path/../Frameworks/${fwdirname}/${fwlibname}'")
#    MESSAGE("")

    MESSAGE("Copying ${fw_srcdir} into bundle...")

    # This command copies the *entire* framework recursively:
    #
#    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory
#      "${fw_srcdir}" "${fw_dstdir}"
#    )

    # This command copies just the main shared lib of the framework:
    # (This technique will not work for frameworks that have necessary
    # resource or auxiliary files...)
    #
    MESSAGE("Copy: ${CMAKE_COMMAND} -E copy \"${fw_src}\"  \"${fw_dstdir}/${fwlibname}\"")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
      "${fw_src}" "${fw_dstdir}/${fwlibname}"
    )

    EXECUTE_PROCESS(COMMAND install_name_tool
      -id "@executable_path/../Frameworks/${fwdirname}/${fwlibname}"
      "${clib_bundle}/Contents/Frameworks/${fwdirname}/${fwlibname}"
    )
    SET(${clib_dstlibs} ${${clib_dstlibs}}
      "${clib_bundle}/Contents/Frameworks/${fwdirname}/${fwlibname}"
    )
    SET(${clib_fixups} ${${clib_fixups}}
      "-change"
      "${clib_libsrc}"
      "@executable_path/../Frameworks/${fwdirname}/${fwlibname}"
    )
  ELSE("${clib_libsrc}" MATCHES ".framework/.*/.*/.*")
    IF("${clib_libsrc}" MATCHES "/")
      SET(clib_libsrcfull "${clib_libsrc}")
    ELSE("${clib_libsrc}" MATCHES "/")
      SET(clib_libsrcfull "${lib_path}/${clib_libsrc}")
      IF(NOT EXISTS "${clib_libsrcfull}")
        MESSAGE(FATAL_ERROR "error: '${clib_libsrcfull}' does not exist...")
      ENDIF(NOT EXISTS "${clib_libsrcfull}")
    ENDIF("${clib_libsrc}" MATCHES "/")

    GET_FILENAME_COMPONENT(dylib_src "${clib_libsrcfull}" ABSOLUTE)
    GET_FILENAME_COMPONENT(dylib_name "${dylib_src}" NAME)
    SET(dylib_dst "${clib_bundle}/Contents/MacOS/${dylib_name}")

#    MESSAGE("dylib_src: ${dylib_src}")
#    MESSAGE("dylib_dst: ${dylib_dst}")
#    MESSAGE("new_name: '@executable_path/${dylib_name}'")

    MESSAGE("Copying ${dylib_src} into bundle...")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
      "${dylib_src}" "${dylib_dst}")
    EXECUTE_PROCESS(COMMAND install_name_tool
      -id "@executable_path/${dylib_name}"
      "${dylib_dst}"
    )
    SET(${clib_dstlibs} ${${clib_dstlibs}}
      "${dylib_dst}"
    )
    SET(${clib_fixups} ${${clib_fixups}}
      "-change"
      "${clib_libsrc}"
      "@executable_path/${dylib_name}"
    )
  ENDIF("${clib_libsrc}" MATCHES ".framework/.*/.*/.*")
ENDMACRO(COPY_LIBRARY_INTO_BUNDLE)


# Copy dependent "nonsystem" libraries into the bundle:
#
MESSAGE("Copying dependent libraries into bundle...")
SET(srclibs ${nonsystem_deps} ${extra_libs})
SET(dstlibs "")
SET(fixups "")
FOREACH(d ${srclibs})
  COPY_LIBRARY_INTO_BUNDLE("${bundle}" "${d}" dstlibs fixups)
ENDFOREACH(d)

MESSAGE("")
MESSAGE("dstlibs='${dstlibs}'")
MESSAGE("")
MESSAGE("fixups='${fixups}'")
MESSAGE("")


# Fixup references to copied libraries in the main bundle executable and in the
# copied libraries themselves:
#
IF(NOT "${fixups}" STREQUAL "")
  MESSAGE("Fixing up references...")
  FOREACH(d ${dstlibs} "${input_file_full}")
    MESSAGE("fixing up references in: '${d}'")
    EXECUTE_PROCESS(COMMAND install_name_tool ${fixups} "${d}")
  ENDFOREACH(d)
  MESSAGE("")
ENDIF(NOT "${fixups}" STREQUAL "")


# List all references to eyeball them and make sure they look right:
#
MESSAGE("Listing references...")
FOREACH(d ${dstlibs} "${input_file_full}")
  EXECUTE_PROCESS(COMMAND otool -L "${d}")
  MESSAGE("")
ENDFOREACH(d)
MESSAGE("")


# Output file:
#
GET_FILENAME_COMPONENT(script_name "${CMAKE_CURRENT_LIST_FILE}" NAME)
FILE(WRITE "${input_file_full}_${script_name}" "# Script \"${CMAKE_CURRENT_LIST_FILE}\" completed.\n")
MESSAGE("")
MESSAGE("# Script \"${CMAKE_CURRENT_LIST_FILE}\" completed.")
MESSAGE("")
