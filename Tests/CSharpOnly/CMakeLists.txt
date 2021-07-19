cmake_minimum_required(VERSION 3.3)
if(POLICY CMP0126)
  cmake_policy(SET CMP0126 NEW)
endif()
# a simple CSharp only test case
project (CSharpOnly CSharp)

# C# does not make any difference between STATIC and SHARED libs
add_library(lib1 STATIC lib1.cs nested/lib1.cs)
#without the source group this test will fail to compile
source_group(nested FILES nested/lib1.cs)
add_library(lib2 SHARED lib2.cs)

add_executable(CSharpOnly csharponly.cs)
target_link_libraries(CSharpOnly lib1 lib2)

add_executable(CSharpConfigSpecific
  $<$<CONFIG:Debug>:config_specific_main_debug.cs>
  $<$<NOT:$<CONFIG:Debug>>:config_specific_main_not_debug.cs>
  $<$<CONFIG:NotAConfig>:config_specific_main_no_exist.cs>
  )

add_custom_target(CSharpCustom ALL SOURCES empty.cs)
add_custom_target(custom.cs ALL DEPENDS empty.txt)
