enable_language(CSharp)
set(CMAKE_CONFIGURATION_TYPES Debug)

set(SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/CSharpSourceGroup/foo.cs
  ${CMAKE_CURRENT_SOURCE_DIR}/CSharpSourceGroup/nested/baz.cs
)

set(IMAGE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/CSharpSourceGroup/Images/empty.bmp
)
# We explicitly don't set a source group for a source in the root level
# because of https://gitlab.kitware.com/cmake/cmake/-/issues/21221
set(RESOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/VsCsharpSourceGroup.png
)


add_library(VsCsharpSourceGroup SHARED ${SRC_FILES} ${IMAGE_FILES} ${RESOURCE_FILES})
source_group("CSharpSourceGroup" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CSharpSourceGroup/foo.cs)
source_group("CSharpSourceGroup/nested" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CSharpSourceGroup/nested/baz.cs)
source_group("Images" FILES ${IMAGE_FILES})

# Test covering CMake Issue 22104.
# Basically there should not be any link tags for files in the binary directory.
include(CSharpUtilities)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CSharpSourceGroup/cmake/AssemblyInfo.cs.in" "Properties/AssemblyInfo.cs")

target_sources(VsCsharpSourceGroup PRIVATE "Properties/AssemblyInfo.cs")
csharp_set_designer_cs_properties("Properties/AssemblyInfo.cs")
