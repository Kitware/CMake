cmake_minimum_required (VERSION 3.23)
project (TestAWT)

find_package (JNI REQUIRED COMPONENTS AWT)
find_package (Java REQUIRED COMPONENTS Runtime Development)

add_library (AWTTgt MODULE awt.cxx)
target_link_libraries (AWTTgt PRIVATE JNI::AWT)

add_library (AWTVar MODULE awt.cxx)
target_include_directories (AWTVar PRIVATE ${JNI_INCLUDE_DIRS})
target_link_libraries (AWTVar PRIVATE ${JNI_LIBRARIES})

enable_testing ()

foreach (test AWTTgt AWTVar)
  add_test (NAME Compile${test} COMMAND ${Java_JAVAC_EXECUTABLE}
    ${TestAWT_SOURCE_DIR}/${test}.java)
  add_test (NAME Remove${test} COMMAND ${CMAKE_COMMAND} -E remove
    ${TestAWT_SOURCE_DIR}/${test}.class)

  add_test (NAME ${test} COMMAND ${Java_JAVA_EXECUTABLE}
    -cp ${TestAWT_SOURCE_DIR}
    -Djava.library.path=$<TARGET_FILE_DIR:${test}>
    ${test}
  )

  set_tests_properties (Compile${test} PROPERTIES FIXTURES_SETUP Init${test})
  set_tests_properties (Remove${test} PROPERTIES FIXTURES_CLEANUP Init${test})
  set_tests_properties (${test} PROPERTIES FIXTURES_REQUIRED Init${test})
endforeach ()
