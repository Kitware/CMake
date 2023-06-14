enable_language(CXX)

set(CMAKE_CXX_STANDARD 11)
find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core Widgets Gui)

if(NOT TARGET Qt${with_qt_version}::uic)
  message(FATAL_ERROR "Qt${with_qt_version}::uic not found")
endif()

add_library(dummy STATIC example_ui.cpp uiA.ui)
target_link_libraries(dummy Qt${with_qt_version}::Core
                            Qt${with_qt_version}::Widgets
                            Qt${with_qt_version}::Gui)

get_target_property(uic_location Qt${with_qt_version}::uic IMPORTED_LOCATION)
set_target_properties(dummy PROPERTIES AUTOUIC_OPTIONS "EXE_PATH=${uic_location}")

add_executable(myuic $<$<CONFIG:Debug>:exe_debug.cpp>
                     $<$<CONFIG:Release>:exe_release.cpp>
                     $<$<CONFIG:RelWithDebInfo>:exe_relwithdebinfo.cpp>
)

set_target_properties(dummy PROPERTIES AUTOUIC_EXECUTABLE $<TARGET_FILE:myuic>
                                       AUTOUIC ON)
