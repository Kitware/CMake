cmake_minimum_required(VERSION 2.8)

project(Qt4Targets)

cmake_policy(SET CMP0020 NEW)

find_package(Qt4 REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(Qt4Targets WIN32 main.cpp)
target_link_libraries(Qt4Targets Qt4::QtGui)

if (WIN32)
  if (TARGET Qt4::QAxServer)
    add_executable(activeqtexe WIN32 activeqtexe.cpp)
    set_property(TARGET activeqtexe PROPERTY QT4_NO_LINK_QTMAIN ON)
    target_link_libraries(activeqtexe Qt4::QAxServer Qt4::QtGui)
  endif()
endif()

qt4_generate_moc(main_gen_test.cpp
  "${CMAKE_CURRENT_BINARY_DIR}/main_gen_test.moc"
  TARGET Qt4GenerateMacroTest
)
add_executable(Qt4GenerateMacroTest WIN32 main_gen_test.cpp "${CMAKE_CURRENT_BINARY_DIR}/main_gen_test.moc")
set_property(TARGET Qt4GenerateMacroTest PROPERTY AUTOMOC OFF)
target_include_directories(Qt4GenerateMacroTest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/interface")
target_link_libraries(Qt4GenerateMacroTest Qt4::QtGui)

qt4_wrap_cpp(moc_file mywrapobject.h
  TARGET Qt4WrapMacroTest
)
add_executable(Qt4WrapMacroTest WIN32 main_wrap_test.cpp ${moc_file})
set_property(TARGET Qt4WrapMacroTest PROPERTY AUTOMOC OFF)
target_include_directories(Qt4WrapMacroTest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/interface")
target_link_libraries(Qt4WrapMacroTest Qt4::QtGui)
