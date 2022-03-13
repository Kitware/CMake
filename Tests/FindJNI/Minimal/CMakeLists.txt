cmake_minimum_required (VERSION 3.23)
project (TestMinimal)

set (CMAKE_CXX_VISIBILITY_PRESET hidden)

find_package (JNI REQUIRED)
find_package (Java REQUIRED COMPONENTS Runtime Development)

add_library (MinimalTgt MODULE minimal.cxx)
target_link_libraries (MinimalTgt PRIVATE JNI::JNI)

add_library (MinimalVar MODULE minimal.cxx)
target_include_directories (MinimalVar PRIVATE ${JNI_INCLUDE_DIRS})
target_link_libraries (MinimalVar PRIVATE ${JNI_LIBRARIES})

enable_testing ()

foreach (test MinimalTgt MinimalVar)
  add_test (NAME Compile${test} COMMAND ${Java_JAVAC_EXECUTABLE}
    ${TestMinimal_SOURCE_DIR}/${test}.java)
  add_test (NAME Remove${test} COMMAND ${CMAKE_COMMAND} -E remove
    ${TestMinimal_SOURCE_DIR}/${test}.class)

  add_test (NAME ${test} COMMAND ${Java_JAVA_EXECUTABLE}
    -cp ${TestMinimal_SOURCE_DIR}
    -Djava.library.path=$<TARGET_FILE_DIR:${test}>
    ${test}
  )

  set_tests_properties (Compile${test} PROPERTIES FIXTURES_SETUP Init${test})
  set_tests_properties (Remove${test} PROPERTIES FIXTURES_CLEANUP Init${test})
  set_tests_properties (${test} PROPERTIES FIXTURES_REQUIRED Init${test})

  set_tests_properties (${test} PROPERTIES PASS_REGULAR_EXPRESSION "^FindJNI\n$")
endforeach ()
