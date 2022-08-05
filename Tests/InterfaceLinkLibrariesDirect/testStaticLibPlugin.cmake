# Logic common to InterfaceLinkLibrariesDirect and ExportImport tests.
set(src ${CMAKE_CURRENT_LIST_DIR})
add_library(testStaticLibWithPlugin STATIC
  ${src}/testStaticLibWithPlugin1.c    # used by testStaticLibPlugin
  ${src}/testStaticLibWithPlugin2.c    # used by testStaticLibPluginExtra
  ${src}/testStaticLibWithPluginBad1.c # link error if not after testStaticLibPlugin
  ${src}/testStaticLibWithPluginBad2.c # link error if not after testStaticLibPluginExtra
  )
add_library(testStaticLibPluginExtra STATIC ${src}/testStaticLibPluginExtra.c)
add_library(testStaticLibPlugin STATIC ${src}/testStaticLibPlugin.c)
target_link_libraries(testStaticLibPlugin PUBLIC testStaticLibWithPlugin testStaticLibPluginExtra)
target_link_libraries(testStaticLibPluginExtra PUBLIC testStaticLibWithPlugin)
set_property(TARGET testStaticLibWithPlugin PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT testStaticLibPlugin)
set_property(TARGET testStaticLibWithPlugin PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE testStaticLibWithPlugin)
