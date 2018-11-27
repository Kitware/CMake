cmake_minimum_required(VERSION 3.10)
project(MocOnly)
include("../AutogenCoreTest.cmake")

add_executable(mocOnly
  main.cpp
  # Test different Q_OBJECT position styles
  StyleA.cpp
  StyleB.cpp
  # Test different moc_/.moc include positions
  IncA.cpp
  IncB.cpp
)
set_property(TARGET mocOnly PROPERTY AUTOMOC ON)
target_link_libraries(mocOnly ${QT_LIBRARIES})
# Add compile definitions with unusual characters
target_compile_definitions(mocOnly PUBLIC "TOKEN=\"hello\;\"" )
