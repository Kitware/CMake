cmake_minimum_required(VERSION 3.20)
project(WrappedFindPackage)

# Wrap the find_package call in a function.
# Test whether AutoMoc can retrieve the Qt version from the moc executable.
function(find_qt_package)
  include("../AutogenCoreTest.cmake")
  set(QT_LIBRARIES "${QT_LIBRARIES}" PARENT_SCOPE)
endfunction()

find_qt_package()

set(CMAKE_AUTOMOC ON)

add_executable(wrappedFindPackage main.cpp)
target_link_libraries(wrappedFindPackage PRIVATE ${QT_LIBRARIES})
