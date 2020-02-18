enable_language(CSharp)
set(CMAKE_CONFIGURATION_TYPES Debug)

set(SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/CSharpSourceGroup/foo.cs
  ${CMAKE_CURRENT_SOURCE_DIR}/CSharpSourceGroup/nested/baz.cs
)

set(IMAGE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/CSharpSourceGroup/Images/empty.bmp
)

add_library(VsCsharpSourceGroup SHARED ${SRC_FILES} ${IMAGE_FILES})
source_group("CSharpSourceGroup" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CSharpSourceGroup/foo.cs)
source_group("CSharpSourceGroup/nested" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CSharpSourceGroup/nested/baz.cs)
source_group("Images" FILES ${IMAGE_FILES})
