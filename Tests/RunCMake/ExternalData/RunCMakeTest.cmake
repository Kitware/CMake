include(RunCMake)

function(run_externaldata_local test)
  run_cmake_command(${test}
    ${CMAKE_COMMAND}
    -DExternalData_ACTION=local
    -DExternalData_OBJECT_STORES=${RunCMake_BINARY_DIR}/${test}-store
    ${ARGN}
    -Dfile=${RunCMake_BINARY_DIR}/${test}-output.txt
    -Dname=${RunCMake_SOURCE_DIR}/CMakeLists.txt
    -P ${RunCMake_SOURCE_DIR}/../../../Modules/ExternalData.cmake
    )
endfunction()

function(run_externaldata_local_link_mode test link_mode)
  string(REPLACE ";" "\\\\;" link_mode_arg "${link_mode}")
  run_externaldata_local(${test}
    "-DExternalData_LINK_MODE=${link_mode_arg}"
    ${ARGN}
    )
endfunction()

run_cmake(BadAlgoMap1)
run_cmake(BadAlgoMap2)
run_cmake(BadArguments)
run_cmake(BadCustom1)
run_cmake(BadCustom2)
run_cmake(BadCustom3)
run_cmake(BadCustom4)
run_cmake(BadHashAlgo1)
run_externaldata_local_link_mode(BadLinkMode "invalid;copy"
  )
run_externaldata_local_link_mode(BadLinkModeNoSymlinks "symlink;copy"
  -DExternalData_NO_SYMLINKS=1
  )
run_externaldata_local_link_mode(GoodLinkModeList "copy;hardlink"
  )
run_cmake(BadOption1)
run_cmake(BadOption2)
run_cmake(BadRecurse1)
run_cmake(BadRecurse2)
run_cmake(BadRecurse3)
run_cmake(BadSeries1)
run_cmake(BadSeries2)
run_cmake(BadSeries3)
run_cmake(Directory1)
run_cmake(Directory2)
run_cmake(Directory3)
run_cmake(Directory4)
run_cmake(Directory5)
run_cmake(LinkContentMD5)
run_cmake(LinkContentSHA1)
run_cmake(LinkDirectory1)
run_cmake(MissingData)
run_cmake(MissingDataWithAssociated)
run_cmake(NoLinkInSource)
run_cmake(NoURLTemplates)
run_cmake(NormalData1)
run_cmake(NormalData2)
run_cmake(NormalData3)
run_cmake(NormalDataSub1)
run_cmake(NotUnderRoot)
run_cmake(ObjectStoreOnly)
run_cmake(Semicolon1)
run_cmake(Semicolon2)
run_cmake(Semicolon3)
run_cmake(SubDirectory1)
