cmake_minimum_required(VERSION 3.3)
project(TestFindOctave CXX)
enable_testing()

find_package(Octave REQUIRED COMPONENTS Development Interpreter)

add_definitions(-DCMAKE_EXPECTED_Octave_VERSION=${Octave_VERSION})

add_executable(test_tgt main.cpp)
target_link_libraries(test_tgt Octave::Octave)
add_test(NAME test_tgt COMMAND test_tgt)

add_test(NAME test_tgt_exe
         COMMAND Octave::Interpreter -q --eval "runtests('.')"
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_var main.cpp)
target_include_directories(test_var PRIVATE ${Octave_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${Octave_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
