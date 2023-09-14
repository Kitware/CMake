enable_language(CXX)

set(CMAKE_CXX_STANDARD 11)
find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core Widgets Gui)

if(NOT TARGET Qt${with_qt_version}::rcc)
  message(FATAL_ERROR "Qt${with_qt_version}::rcc not found")
endif()

add_library(dummy STATIC example.cpp data.qrc)
target_link_libraries(dummy Qt${with_qt_version}::Core
                            Qt${with_qt_version}::Widgets
                            Qt${with_qt_version}::Gui)

get_target_property(rcc_location Qt${with_qt_version}::rcc IMPORTED_LOCATION)
set_target_properties(dummy PROPERTIES AUTORCC_OPTIONS "EXE_PATH=${rcc_location}")

add_executable(myrcc $<$<CONFIG:Debug>:exe_debug.cpp>
                     $<$<CONFIG:Release>:exe_release.cpp>
                     $<$<CONFIG:RelWithDebInfo>:exe_relwithdebinfo.cpp>
)

set_target_properties(dummy PROPERTIES AUTORCC_EXECUTABLE $<TARGET_FILE:myrcc>
                                       AUTORCC ON)
