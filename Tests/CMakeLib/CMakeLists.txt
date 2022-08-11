include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMake_BINARY_DIR}/Source
  ${CMake_SOURCE_DIR}/Source
  ${CMake_SOURCE_DIR}/Source/CTest
  )

set(CMakeLib_TESTS
  testArgumentParser.cxx
  testCTestBinPacker.cxx
  testCTestResourceAllocator.cxx
  testCTestResourceSpec.cxx
  testCTestResourceGroups.cxx
  testGccDepfileReader.cxx
  testGeneratedFileStream.cxx
  testJSONHelpers.cxx
  testRST.cxx
  testRange.cxx
  testOptional.cxx
  testString.cxx
  testStringAlgorithms.cxx
  testSystemTools.cxx
  testUTF8.cxx
  testXMLParser.cxx
  testXMLSafe.cxx
  testFindPackageCommand.cxx
  testUVProcessChain.cxx
  testUVRAII.cxx
  testUVStreambuf.cxx
  testCMExtMemory.cxx
  testCMExtAlgorithm.cxx
  testCMExtEnumSet.cxx
  )
if (CMake_TEST_FILESYSTEM_PATH OR NOT CMake_HAVE_CXX_FILESYSTEM)
  list(APPEND CMakeLib_TESTS testCMFilesystemPath.cxx)
endif()

add_executable(testUVProcessChainHelper testUVProcessChainHelper.cxx)

set(testRST_ARGS ${CMAKE_CURRENT_SOURCE_DIR})
set(testUVProcessChain_ARGS $<TARGET_FILE:testUVProcessChainHelper>)
set(testUVStreambuf_ARGS $<TARGET_FILE:cmake>)
set(testCTestResourceSpec_ARGS ${CMAKE_CURRENT_SOURCE_DIR})
set(testGccDepfileReader_ARGS ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
  list(APPEND CMakeLib_TESTS
    testVisualStudioSlnParser.cxx
    )
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testVisualStudioSlnParser.h.in
                 ${CMAKE_CURRENT_BINARY_DIR}/testVisualStudioSlnParser.h @ONLY)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testXMLParser.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/testXMLParser.h @ONLY)

create_test_sourcelist(CMakeLib_TEST_SRCS CMakeLibTests.cxx ${CMakeLib_TESTS})
add_executable(CMakeLibTests ${CMakeLib_TEST_SRCS})
target_link_libraries(CMakeLibTests CMakeLib CTestLib)

set_property(TARGET CMakeLibTests PROPERTY C_CLANG_TIDY "")
set_property(TARGET CMakeLibTests PROPERTY CXX_CLANG_TIDY "")

add_executable(testEncoding testEncoding.cxx)
target_link_libraries(testEncoding cmsys)

foreach(testfile ${CMakeLib_TESTS})
  get_filename_component(test "${testfile}" NAME_WE)
  add_test(NAME CMakeLib.${test} COMMAND CMakeLibTests ${test} ${${test}_ARGS})
endforeach()

if(TEST_CompileCommandOutput)
  add_executable(runcompilecommands run_compile_commands.cxx)
  target_link_libraries(runcompilecommands CMakeLib)
endif()

add_subdirectory(PseudoMemcheck)

add_executable(testAffinity testAffinity.cxx)
target_link_libraries(testAffinity CMakeLib)
