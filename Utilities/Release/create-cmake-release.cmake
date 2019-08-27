if(NOT DEFINED CMAKE_CREATE_VERSION)
  set(CMAKE_CREATE_VERSION "release")
  message("Using default value of 'release' for CMAKE_CREATE_VERSION")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/logs)

function(write_rel_shell_script filename script)
  file(WRITE ${filename} "#!/usr/bin/env bash
\"${CMAKE_COMMAND}\" -DCMAKE_CREATE_VERSION=${CMAKE_CREATE_VERSION} -DCMAKE_DOC_TARBALL=\"${CMAKE_DOC_TARBALL}\" -P \"${CMAKE_CURRENT_LIST_DIR}/${script}.cmake\" < /dev/null 2>&1 | tee \"${CMAKE_CURRENT_SOURCE_DIR}/logs/${script}-${CMAKE_CREATE_VERSION}.log\"
")
  execute_process(COMMAND chmod a+x ${filename})
endfunction()

function(write_docs_shell_script filename)
  find_program(SPHINX_EXECUTABLE
    NAMES sphinx-build sphinx-build.py
    DOC "Sphinx Documentation Builder (sphinx-doc.org)"
    )
  if(NOT SPHINX_EXECUTABLE)
    message(FATAL_ERROR "SPHINX_EXECUTABLE (sphinx-build) is not found!")
  endif()

  set(name cmake-${CMAKE_CREATE_VERSION}-docs)
  file(WRITE "${filename}" "#!/usr/bin/env bash

name=${name} &&
inst=\"\$PWD/\$name\"
(GIT_WORK_TREE=x git archive --prefix=\${name}-src/ ${CMAKE_CREATE_VERSION}) | tar x &&
rm -rf \${name}-build &&
mkdir \${name}-build &&
cd \${name}-build &&
\"${CMAKE_COMMAND}\" ../\${name}-src/Utilities/Sphinx \\
  -DCMAKE_INSTALL_PREFIX=\"\$inst/\" \\
  -DCMAKE_DOC_DIR=doc/cmake \\
  -DSPHINX_EXECUTABLE=\"${SPHINX_EXECUTABLE}\" \\
  -DSPHINX_HTML=ON -DSPHINX_MAN=ON -DSPHINX_QTHELP=ON &&
make install &&
cd .. &&
tar czf \${name}.tar.gz \${name} ||
echo 'Failed to create \${name}.tar.gz'
")
  execute_process(COMMAND chmod a+x ${filename})
  set(CMAKE_DOC_TARBALL "${name}.tar.gz" PARENT_SCOPE)
endfunction()

write_docs_shell_script("create-${CMAKE_CREATE_VERSION}-docs.sh")
write_rel_shell_script("create-${CMAKE_CREATE_VERSION}-macos.sh"   osx_release    ) # macOS x86_64
write_rel_shell_script("create-${CMAKE_CREATE_VERSION}-win64.sh"   win64_release  ) # Windows x64
write_rel_shell_script("create-${CMAKE_CREATE_VERSION}-win32.sh"   win32_release  ) # Windows x86

message("Build docs first and then build for each platform:
 ./create-${CMAKE_CREATE_VERSION}-docs.sh    &&
 ./create-${CMAKE_CREATE_VERSION}-macos.sh   &&
 ./create-${CMAKE_CREATE_VERSION}-win64.sh   &&
 ./create-${CMAKE_CREATE_VERSION}-win32.sh   &&
 echo done
")
