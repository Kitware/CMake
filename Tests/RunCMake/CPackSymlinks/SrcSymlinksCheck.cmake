set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(tarball ${dir}/SrcSymlinks-0.1-Source.tar.gz)
set(extrdir ${dir}/SrcSymlinks-0.1-Source)

message(STATUS "Extracting ${tarball} in ${dir}...")
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvf ${tarball}
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE output
  WORKING_DIRECTORY ${dir})
message(STATUS "result='${result}'")
message(STATUS "output='${output}'")

if(NOT ${result} EQUAL 0)
  message(FATAL_ERROR "Cannot unpack source tarball")
endif()

if(NOT EXISTS ${extrdir}/dirlink/src.h)
  message(FATAL_ERROR "${extrdir}/dirlink/src.h not found")
endif()
