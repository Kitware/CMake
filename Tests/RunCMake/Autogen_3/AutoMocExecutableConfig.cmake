include(MocExample.cmake)

if(NOT TARGET Qt${with_qt_version}::moc)
  message(FATAL_ERROR "Qt${with_qt_version}::moc not found")
endif()

get_target_property(moc_location Qt${with_qt_version}::moc IMPORTED_LOCATION)
set_target_properties(dummy PROPERTIES AUTOMOC_MOC_OPTIONS "EXE_PATH=${moc_location}")

add_executable(mymoc $<$<CONFIG:Debug>:../Autogen_common/exe_debug.cpp>
                     $<$<CONFIG:Release>:../Autogen_common/exe_release.cpp>
                     $<$<CONFIG:RelWithDebInfo>:../Autogen_common/exe_relwithdebinfo.cpp>
)

set_target_properties(dummy PROPERTIES AUTOMOC_EXECUTABLE $<TARGET_FILE:mymoc>)
