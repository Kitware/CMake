if(NOT DEFINED CMAKE_VERSION)
  message(FATAL_ERROR "CMAKE_VERSION not defined")
endif(NOT DEFINED CMAKE_VERSION)

set(RELEASE_SCRIPTS
  dashmacmini2_release.cmake  # Mac Darwin universal
  dashsun1_release.cmake      # SunOS
  destiny_release.cmake       # HPUX
  magrathea_release.cmake     # Linux
  dashsgi1_release.cmake      # IRIX
  dashsgi1_release64.cmake    # IRIX 64
  vogon_release.cmake         # Windows
  v20n17_aix_release.cmake    # AIX 5.3
  vogon_cygwin.cmake          # Cygwin
#  r36n11_aix_release.cmake    # AIX 5.3
#  r15n65_aix_release.cmake    # AIX 5.2
)

file(WRITE create-${CMAKE_VERSION}.sh "#!/bin/sh")
make_directory(${CMAKE_CURRENT_SOURCE_DIR}/logs)

foreach(f ${RELEASE_SCRIPTS})
  file(APPEND create-${CMAKE_VERSION}.sh
    "
${CMAKE_COMMAND} -DCMAKE_VERSION=${CMAKE_VERSION} -P ${CMAKE_ROOT}/Utilities/Release/${f} < /dev/null >& ${CMAKE_CURRENT_SOURCE_DIR}/logs/${f}-${CMAKE_VERSION}.log &
 xterm -geometry 80x10 -sb -sl 2000 -T ${f}-${CMAKE_VERSION}.log -e tail -f  ${CMAKE_CURRENT_SOURCE_DIR}/logs/${f}-${CMAKE_VERSION}.log&")
endforeach(f)
execute_process(COMMAND chmod a+x create-${CMAKE_VERSION}.sh)
message("Run ./create-${CMAKE_VERSION}.sh")


