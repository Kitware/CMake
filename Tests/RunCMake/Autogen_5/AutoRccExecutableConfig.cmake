include(RccExample.cmake)

if(NOT TARGET Qt${with_qt_version}::rcc)
  message(FATAL_ERROR "Qt${with_qt_version}::rcc not found")
endif()

get_target_property(rcc_location Qt${with_qt_version}::rcc IMPORTED_LOCATION)
set_target_properties(dummy PROPERTIES AUTORCC_OPTIONS "EXE_PATH=${rcc_location}")

add_executable(myrcc $<$<CONFIG:Debug>:../Autogen_common/exe_debug.cpp>
                     $<$<CONFIG:Release>:../Autogen_common/exe_release.cpp>
                     $<$<CONFIG:RelWithDebInfo>:../Autogen_common/exe_relwithdebinfo.cpp>
)

set_target_properties(dummy PROPERTIES AUTORCC_EXECUTABLE $<TARGET_FILE:myrcc>)
