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
if(NOT DEFINED input_file)
  message(FATAL_ERROR "
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
endif(NOT DEFINED input_file)
message("ingest ${input_file}")
set(eol_char "E")

if(APPLE)
  set(dep_tool "otool")
  set(dep_cmd_args "-L")
  set(dep_regex "^\t([^\t]+) \\(compatibility version ([0-9]+.[0-9]+.[0-9]+), current version ([0-9]+.[0-9]+.[0-9]+)\\)${eol_char}$")
endif(APPLE)

message("")
message("# Script \"${CMAKE_CURRENT_LIST_FILE}\" running...")
message("")
message("input_file: '${input_file}'")
message("extra_libs: '${extra_libs}'")
message("lib_path: '${lib_path}'")
message("")

get_filename_component(input_file_full "${input_file}" ABSOLUTE)
message("input_file_full: '${input_file_full}'")

get_filename_component(bundle "${input_file_full}/../../.." ABSOLUTE)
message("bundle: '${bundle}'")


find_program(dep_cmd ${dep_tool})

# find the full path to the framework in path set the result
# in pathout
macro(find_framework_full_path path pathout)
  set(${pathout} "${path}")
  if(NOT EXISTS "${path}")
    set(FRAMEWORK_SEARCH "/Library/Frameworks"
      "/System/Library/Frameworks" )
    set(__FOUND FALSE)
    foreach(f ${FRAMEWORK_SEARCH})
      set(newd "${f}/${path}")
      if(EXISTS "${newd}" AND NOT __FOUND)
        set(${pathout} "${newd}")
        set(__FOUND TRUE)
      endif(EXISTS "${newd}" AND NOT __FOUND)
    endforeach(f)
  endif(NOT EXISTS "${path}")
endmacro(find_framework_full_path)


macro(append_unique au_list_var au_value)
  set(${au_list_var} ${${au_list_var}} "${au_value}")
endmacro(append_unique)


macro(gather_dependents gd_target gd_dependents_var)
  execute_process(
    COMMAND ${dep_cmd} ${dep_cmd_args} ${gd_target}
    OUTPUT_VARIABLE dep_tool_ov
    )

  string(REGEX REPLACE ";" "\\\\;" dep_candidates "${dep_tool_ov}")
  string(REGEX REPLACE "\n" "${eol_char};" dep_candidates "${dep_candidates}")

  set(${gd_dependents_var} "")

  foreach(candidate ${dep_candidates})
  if("${candidate}" MATCHES "${dep_regex}")
    string(REGEX REPLACE "${dep_regex}" "\\1" raw_item "${candidate}")
    string(REGEX REPLACE "${dep_regex}" "\\2" raw_compat_version "${candidate}")
    string(REGEX REPLACE "${dep_regex}" "\\3" raw_current_version "${candidate}")

    set(item "${raw_item}")

    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\1" compat_major_version "${raw_compat_version}")
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\2" compat_minor_version "${raw_compat_version}")
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" compat_patch_version "${raw_compat_version}")

    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\1" current_major_version "${raw_current_version}")
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\2" current_minor_version "${raw_current_version}")
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" current_patch_version "${raw_current_version}")

    #message("${raw_item} - compat ${raw_compat_version} - current ${raw_current_version}")
    append_unique("${gd_dependents_var}" "${item}")
  else("${candidate}" MATCHES "${dep_regex}")
    if("${candidate}" STREQUAL "${gd_target}:${eol_char}")
      #message("info: ignoring target name...")
    else("${candidate}" STREQUAL "${gd_target}:${eol_char}")
      message("error: candidate='${candidate}'")
    endif("${candidate}" STREQUAL "${gd_target}:${eol_char}")
  endif("${candidate}" MATCHES "${dep_regex}")
  endforeach(candidate)
endmacro(gather_dependents)


message("Gathering dependent libraries for '${input_file_full}'...")
gather_dependents("${input_file_full}" deps)
message("")


# Order lexicographically:
#
list(SORT deps)


# Split into separate lists, "system" "embedded" and "nonsystem" libraries.
# System libs are assumed to be available on all target runtime Macs and do not
# need to be copied/fixed-up by this script. Embedded libraries are assumed to
# be in the bundle and fixed-up already. Only non-system, non-embedded libs
# need copying and fixing up...
#
set(system_deps "")
set(embedded_deps "")
set(nonsystem_deps "")

foreach(d ${deps})
  set(d_is_embedded_lib 0)
  set(d_is_system_lib 0)

  if("${d}" MATCHES "^(/System/Library|/usr/lib)")
    set(d_is_system_lib 1)
  else("${d}" MATCHES "^(/System/Library|/usr/lib)")
    if("${d}" MATCHES "^@executable_path")
      set(d_is_embedded_lib 1)
    endif("${d}" MATCHES "^@executable_path")
  endif("${d}" MATCHES "^(/System/Library|/usr/lib)")

  if(d_is_system_lib)
    set(system_deps ${system_deps} "${d}")
  else(d_is_system_lib)
    if(d_is_embedded_lib)
      set(embedded_deps ${embedded_deps} "${d}")
    else(d_is_embedded_lib)
      set(nonsystem_deps ${nonsystem_deps} "${d}")
    endif(d_is_embedded_lib)
  endif(d_is_system_lib)
endforeach(d)

message("")
message("system_deps:")
foreach(d ${system_deps})
  message("${d}")
endforeach(d ${system_deps})

message("")
message("embedded_deps:")
foreach(d ${embedded_deps})
  message("${d}")
endforeach(d ${embedded_deps})

message("")
message("nonsystem_deps:")
foreach(d ${nonsystem_deps})
  message("${d}")
endforeach(d ${nonsystem_deps})

message("")


macro(copy_library_into_bundle clib_bundle clib_libsrc clib_dstlibs clib_fixups)
  #
  # If the source library is a framework, copy just the shared lib bit of the framework
  # into the bundle under "${clib_bundle}/Contents/Frameworks" - if it is just a dylib
  # copy it into the same directory with the main bundle executable under
  # "${clib_bundle}/Contents/MacOS"
  #
  if("${clib_libsrc}" MATCHES ".framework/.*/.*/.*")
    # make sure clib_libsrc is a full path to the framework as a framework
    # maybe linked in with relative paths in some cases
    find_framework_full_path("${clib_libsrc}" fw_full_src)
    get_filename_component(fw_src "${fw_full_src}" ABSOLUTE)
    get_filename_component(fw_srcdir "${clib_libsrc}/../../.." ABSOLUTE)
    get_filename_component(fwdirname "${fw_srcdir}" NAME)
    string(REGEX REPLACE "^(.*)\\.framework$" "\\1" fwname "${fwdirname}")
    string(REGEX REPLACE "^.*/${fwname}\\.framework/(.*)$" "\\1" fwlibname "${clib_libsrc}")
    set(fw_dstdir "${clib_bundle}/Contents/Frameworks/${fwdirname}")

#     message("")
#     message("fwdirname: '${fwdirname}'")
#     message("fwname: '${fwname}'")
#     message("fwlibname: '${fwlibname}'")
#     message("fw_src: '${fw_src}'")
#     message("fw_srcdir: '${fw_srcdir}'")
#     message("fw_dstdir: '${fw_dstdir}'")
#     message("new_name: '@executable_path/../Frameworks/${fwdirname}/${fwlibname}'")
#     message("")

    message("Copying ${fw_srcdir} into bundle...")

    # This command copies the *entire* framework recursively:
    #
#    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
#      "${fw_srcdir}" "${fw_dstdir}"
#    )

    # This command copies just the main shared lib of the framework:
    # (This technique will not work for frameworks that have necessary
    # resource or auxiliary files...)
    #
    message("fw_src = [${fw_src}]   fw_full_src = [${fw_full_src}]")
    message("Copy: ${CMAKE_COMMAND} -E copy \"${fw_src}\"  \"${fw_dstdir}/${fwlibname}\"")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy
      "${fw_src}" "${fw_dstdir}/${fwlibname}"
    )

    execute_process(COMMAND install_name_tool
      -id "@executable_path/../Frameworks/${fwdirname}/${fwlibname}"
      "${clib_bundle}/Contents/Frameworks/${fwdirname}/${fwlibname}"
    )
    set(${clib_dstlibs} ${${clib_dstlibs}}
      "${clib_bundle}/Contents/Frameworks/${fwdirname}/${fwlibname}"
    )
    set(${clib_fixups} ${${clib_fixups}}
      "-change"
      "${clib_libsrc}"
      "@executable_path/../Frameworks/${fwdirname}/${fwlibname}"
    )
  else("${clib_libsrc}" MATCHES ".framework/.*/.*/.*")
    if("${clib_libsrc}" MATCHES "/")
      set(clib_libsrcfull "${clib_libsrc}")
    else("${clib_libsrc}" MATCHES "/")
      set(clib_libsrcfull "${lib_path}/${clib_libsrc}")
      if(NOT EXISTS "${clib_libsrcfull}")
        message(FATAL_ERROR "error: '${clib_libsrcfull}' does not exist...")
      endif(NOT EXISTS "${clib_libsrcfull}")
    endif("${clib_libsrc}" MATCHES "/")

    get_filename_component(dylib_src "${clib_libsrcfull}" ABSOLUTE)
    get_filename_component(dylib_name "${dylib_src}" NAME)
    set(dylib_dst "${clib_bundle}/Contents/MacOS/${dylib_name}")

#    message("dylib_src: ${dylib_src}")
#    message("dylib_dst: ${dylib_dst}")
#    message("new_name: '@executable_path/${dylib_name}'")

    message("Copying ${dylib_src} into bundle...")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy
      "${dylib_src}" "${dylib_dst}")
    execute_process(COMMAND install_name_tool
      -id "@executable_path/${dylib_name}"
      "${dylib_dst}"
    )
    set(${clib_dstlibs} ${${clib_dstlibs}}
      "${dylib_dst}"
    )
    set(${clib_fixups} ${${clib_fixups}}
      "-change"
      "${clib_libsrc}"
      "@executable_path/${dylib_name}"
    )
  endif("${clib_libsrc}" MATCHES ".framework/.*/.*/.*")
endmacro(copy_library_into_bundle)


# Copy dependent "nonsystem" libraries into the bundle:
#
message("Copying dependent libraries into bundle...")
set(srclibs ${nonsystem_deps} ${extra_libs})
set(dstlibs "")
set(fixups "")
foreach(d ${srclibs})
  message("copy it --- ${d}")
  copy_library_into_bundle("${bundle}" "${d}" dstlibs fixups)
endforeach(d)

message("")
message("dstlibs='${dstlibs}'")
message("")
message("fixups='${fixups}'")
message("")


# Fixup references to copied libraries in the main bundle executable and in the
# copied libraries themselves:
#
if(NOT "${fixups}" STREQUAL "")
  message("Fixing up references...")
  foreach(d ${dstlibs} "${input_file_full}")
    message("fixing up references in: '${d}'")
    execute_process(COMMAND install_name_tool ${fixups} "${d}")
  endforeach(d)
  message("")
endif(NOT "${fixups}" STREQUAL "")


# List all references to eyeball them and make sure they look right:
#
message("Listing references...")
foreach(d ${dstlibs} "${input_file_full}")
  execute_process(COMMAND otool -L "${d}")
  message("")
endforeach(d)
message("")


# Output file:
#
#get_filename_component(script_name "${CMAKE_CURRENT_LIST_FILE}" NAME)
#file(WRITE "${input_file_full}_${script_name}" "# Script \"${CMAKE_CURRENT_LIST_FILE}\" completed.\n")
message("")
message("# Script \"${CMAKE_CURRENT_LIST_FILE}\" completed.")
message("")
