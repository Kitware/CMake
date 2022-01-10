cmake_minimum_required(VERSION 3.16)
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
# XXX(xcode-per-cfg-src): Drop the NO_PER_CONFIG_SOURCES exclusion
# when the Xcode generator supports per-config sources.
if(NOT NO_PER_CONFIG_SOURCES)
  target_sources(mocOnly PRIVATE
    "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/CfgDebug.cpp>"
    "$<$<NOT:$<CONFIG:Debug>>:${CMAKE_CURRENT_SOURCE_DIR}/CfgOther.cpp>"
    )
  target_compile_definitions(mocOnly PRIVATE
    "$<$<CONFIG:Debug>:HAVE_CFG_DEBUG>"
    "$<$<NOT:$<CONFIG:Debug>>:HAVE_CFG_OTHER>"
    )
endif()
set_property(TARGET mocOnly PROPERTY AUTOMOC ON)
target_link_libraries(mocOnly ${QT_LIBRARIES})
# Add compile definitions with unusual characters
target_compile_definitions(mocOnly PUBLIC "TOKEN=\"hello\;\"" )
