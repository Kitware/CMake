cmake_minimum_required (VERSION 3.23)
project (TestJVM)

find_package (JNI REQUIRED COMPONENTS JVM)
find_package (Java REQUIRED COMPONENTS Runtime Development)

add_library (JVMTgt MODULE jvm.cxx)
target_link_libraries (JVMTgt PRIVATE JNI::JVM)

add_library (JVMVar MODULE jvm.cxx)
target_include_directories (JVMVar PRIVATE ${JNI_INCLUDE_DIRS})
target_link_libraries (JVMVar PRIVATE ${JNI_LIBRARIES})

enable_testing ()

foreach (test JVMTgt JVMVar)
  add_test (NAME Compile${test} COMMAND ${Java_JAVAC_EXECUTABLE}
    ${TestJVM_SOURCE_DIR}/${test}.java)
  add_test (NAME Remove${test} COMMAND ${CMAKE_COMMAND} -E remove
    ${TestJVM_SOURCE_DIR}/${test}.class)

  add_test (NAME ${test} COMMAND ${Java_JAVA_EXECUTABLE}
    -cp ${TestJVM_SOURCE_DIR}
    -Djava.library.path=$<TARGET_FILE_DIR:${test}>
    ${test}
  )

  set_tests_properties (Compile${test} PROPERTIES FIXTURES_SETUP Init${test})
  set_tests_properties (Remove${test} PROPERTIES FIXTURES_CLEANUP Init${test})
  set_tests_properties (${test} PROPERTIES FIXTURES_REQUIRED Init${test})
endforeach ()
