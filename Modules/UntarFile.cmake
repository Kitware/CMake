#
# Use 'cmake -Dfilename=${tar_or_tgz_file} -Dtmp=${tmp_directory} -Ddirectory=${final_directory}
#   -P UntarFile.cmake' to call this script...
#
if(NOT DEFINED filename)
  message(FATAL_ERROR "error: required variable 'filename' not defined...")
endif()

if(NOT DEFINED tmp)
  message(FATAL_ERROR "error: required variable 'tmp' not defined...")
endif()

if(NOT DEFINED directory)
  message(FATAL_ERROR "error: required variable 'directory' not defined...")
endif()

if(NOT DEFINED args)
  if(filename MATCHES ".tar$")
    set(args xf)
  endif()

  if(filename MATCHES ".tgz$")
    set(args xfz)
  endif()

  if(filename MATCHES ".tar.gz$")
    set(args xfz)
  endif()
endif()


# Make file names absolute:
#
get_filename_component(filename "${filename}" ABSOLUTE)
get_filename_component(tmp "${tmp}" ABSOLUTE)
get_filename_component(directory "${directory}" ABSOLUTE)
message(STATUS "filename='${filename}'")
message(STATUS "tmp='${tmp}'")
message(STATUS "directory='${directory}'")


# Prepare a space for untarring:
#
#message(STATUS "info: creating empty subdir of '${tmp}'...")
set(i 1)
while(EXISTS "${tmp}/untar${i}")
  math(EXPR i "${i} + 1")
endwhile()
set(ut_dir "${tmp}/untar${i}")
message(STATUS "ut_dir='${ut_dir}'")
file(MAKE_DIRECTORY "${ut_dir}")


# Untar it:
#
#message(STATUS "info: untarring '${filename}' in '${ut_dir}' with '${args}'...")
execute_process(COMMAND ${CMAKE_COMMAND} -E tar ${args} ${filename}
  WORKING_DIRECTORY ${ut_dir}
  RESULT_VARIABLE rv)

if(NOT rv EQUAL 0)
  message(FATAL_ERROR "error: untar of '${filename}' failed")
endif()


# Analyze what came out of the tar file:
#
file(GLOB contents "${ut_dir}/*")
list(LENGTH contents n)
if(NOT n EQUAL 1 OR NOT IS_DIRECTORY "${contents}")
  set(contents "${ut_dir}")
endif()


# Copy "the one" directory to the final directory:
#
file(COPY "${contents}/" DESTINATION ${directory})


# Clean up:
#
file(REMOVE_RECURSE "${ut_dir}")
