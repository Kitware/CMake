cmake_minimum_required(VERSION 3.10)
project(UicOnly)
include("../AutogenTest.cmake")

# Test AUTOUIC being enabled only
# The moc is provided by the Qt macro
qtx_wrap_cpp(uicOnlyMoc uiconly.h)
add_executable(uicOnly uiconly.cpp ${uicOnlyMoc})
set_property(TARGET uicOnly PROPERTY AUTOUIC ON)
target_link_libraries(uicOnly ${QT_LIBRARIES})
