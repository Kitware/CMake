if(NOT DEFINED CMAKE_CREATE_VERSION)
  set(CMAKE_CREATE_VERSION "release")
  message("Using default value of 'release' for CMAKE_CREATE_VERSION")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/logs)

set(RELEASE_SCRIPTS_BATCH_1
  dash2win64_release.cmake    # Windows
  dashmacmini2_release.cmake  # Mac Darwin universal ppc;i386
  dashmacmini5_release.cmake  # Mac Darwin64 universal x86_64;i386
  magrathea_release.cmake     # Linux
  v20n250_aix_release.cmake   # AIX 5.3
  ferrari_sgi64_release.cmake # IRIX 64
  ferrari_sgi_release.cmake   # IRIX
)

set(RELEASE_SCRIPTS_BATCH_2
  dash2win64_cygwin.cmake     # Cygwin
)

function(write_batch_shell_script filename)
  set(scripts ${ARGN})
  set(i 0)
  file(WRITE ${filename} "#!/bin/bash")
  foreach(f ${scripts})
    math(EXPR x "420*(${i}/4)")
    math(EXPR y "160*(${i}%4)")
    file(APPEND ${filename}
    "
${CMAKE_COMMAND} -DCMAKE_CREATE_VERSION=${CMAKE_CREATE_VERSION} -P ${CMAKE_ROOT}/Utilities/Release/${f} < /dev/null >& ${CMAKE_CURRENT_SOURCE_DIR}/logs/${f}-${CMAKE_CREATE_VERSION}.log &
xterm -geometry 64x6+${x}+${y} -sb -sl 2000 -T ${f}-${CMAKE_CREATE_VERSION}.log -e tail -f  ${CMAKE_CURRENT_SOURCE_DIR}/logs/${f}-${CMAKE_CREATE_VERSION}.log&
")
    math(EXPR i "${i}+1")
  endforeach(f)
  execute_process(COMMAND chmod a+x ${filename})
endfunction()

write_batch_shell_script("create-${CMAKE_CREATE_VERSION}-batch1.sh" ${RELEASE_SCRIPTS_BATCH_1})
write_batch_shell_script("create-${CMAKE_CREATE_VERSION}-batch2.sh" ${RELEASE_SCRIPTS_BATCH_2})

message("Run ./create-${CMAKE_CREATE_VERSION}-batch1.sh, then after all those builds complete, run ./create-${CMAKE_CREATE_VERSION}-batch2.sh")
