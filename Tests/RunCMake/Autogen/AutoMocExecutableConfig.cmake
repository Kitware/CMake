enable_language(CXX)

set(CMAKE_CXX_STANDARD 11)
find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core Widgets Gui)

if(NOT TARGET Qt${with_qt_version}::moc)
  message(FATAL_ERROR "Qt${with_qt_version}::moc not found")
endif()

add_library(dummy STATIC example.cpp)
target_link_libraries(dummy Qt${with_qt_version}::Core
                            Qt${with_qt_version}::Widgets
                            Qt${with_qt_version}::Gui)

get_target_property(moc_location Qt${with_qt_version}::moc IMPORTED_LOCATION)
set_target_properties(dummy PROPERTIES AUTOMOC_MOC_OPTIONS "EXE_PATH=${moc_location}")

add_executable(mymoc $<$<CONFIG:Debug>:exe_debug.cpp>
                     $<$<CONFIG:Release>:exe_release.cpp>
                     $<$<CONFIG:RelWithDebInfo>:exe_relwithdebinfo.cpp>
)

set_target_properties(dummy PROPERTIES AUTOMOC_EXECUTABLE $<TARGET_FILE:mymoc>
                                       AUTOMOC ON)
