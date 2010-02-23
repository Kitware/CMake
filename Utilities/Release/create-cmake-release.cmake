if(NOT DEFINED CMAKE_CREATE_VERSION)
  message(FATAL_ERROR "CMAKE_CREATE_VERSION not defined")
endif(NOT DEFINED CMAKE_CREATE_VERSION)

set(RELEASE_SCRIPTS
  dashmacmini2_release.cmake  # Mac Darwin universal
  dashsun1_release.cmake      # SunOS
  destiny_release.cmake       # HPUX
  magrathea_release.cmake     # Linux
  dash2win64_release.cmake    # Windows
#  dash2win64_cygwin.cmake     # Cygwin
#  blight_cygwin.cmake     # Cygwin
  v20n250_aix_release.cmake    # AIX 5.3
#  vogon_cygwin.cmake          # Cygwin
  ferrari_sgi64_release.cmake # IRIX 64
  ferrari_sgi_release.cmake   # IRIX 64
#  r36n11_aix_release.cmake    # AIX 5.3
#  r15n65_aix_release.cmake    # AIX 5.2
)

file(WRITE create-${CMAKE_CREATE_VERSION}.sh "#!/bin/bash")
make_directory(${CMAKE_CURRENT_SOURCE_DIR}/logs)

foreach(f ${RELEASE_SCRIPTS})
  file(APPEND create-${CMAKE_CREATE_VERSION}.sh
    "
${CMAKE_COMMAND} -DCMAKE_CREATE_VERSION=${CMAKE_CREATE_VERSION} -P ${CMAKE_ROOT}/Utilities/Release/${f} < /dev/null >& ${CMAKE_CURRENT_SOURCE_DIR}/logs/${f}-${CMAKE_CREATE_VERSION}.log &
 xterm -geometry 80x10 -sb -sl 2000 -T ${f}-${CMAKE_CREATE_VERSION}.log -e tail -f  ${CMAKE_CURRENT_SOURCE_DIR}/logs/${f}-${CMAKE_CREATE_VERSION}.log&")
endforeach(f)
execute_process(COMMAND chmod a+x create-${CMAKE_CREATE_VERSION}.sh)
message("Run ./create-${CMAKE_CREATE_VERSION}.sh")


