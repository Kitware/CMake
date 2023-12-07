include(MocExample.cmake)

if(NOT TARGET Qt${with_qt_version}::moc)
  message(FATAL_ERROR "Qt${with_qt_version}::moc not found")
endif()

get_target_property(moc_location Qt${with_qt_version}::moc IMPORTED_LOCATION)
set_target_properties(dummy PROPERTIES AUTOMOC_MOC_OPTIONS "EXE_PATH=${moc_location}")

add_executable(mymoc $<$<CONFIG:Debug>:exe_debug.cpp>
                     $<$<CONFIG:Release>:exe_release.cpp>
                     $<$<CONFIG:RelWithDebInfo>:exe_relwithdebinfo.cpp>
)

set_target_properties(dummy PROPERTIES AUTOMOC_EXECUTABLE $<TARGET_FILE:mymoc>)
