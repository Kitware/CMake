include(UicExample.cmake)

if(NOT TARGET Qt${with_qt_version}::uic)
  message(FATAL_ERROR "Qt${with_qt_version}::uic not found")
endif()

get_target_property(uic_location Qt${with_qt_version}::uic IMPORTED_LOCATION)
set_target_properties(dummy PROPERTIES AUTOUIC_OPTIONS "EXE_PATH=${uic_location}")

add_executable(myuic $<$<CONFIG:Debug>:exe_debug.cpp>
                     $<$<CONFIG:Release>:exe_release.cpp>
                     $<$<CONFIG:RelWithDebInfo>:exe_relwithdebinfo.cpp>
)

set_target_properties(dummy PROPERTIES AUTOUIC_EXECUTABLE $<TARGET_FILE:myuic>)
