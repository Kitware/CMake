# Logic common to InterfaceLinkLibrariesDirect and ExportImport tests.
set(src ${CMAKE_CURRENT_LIST_DIR})
add_executable(testExeWithPluginHelper ${src}/testExeWithPluginHelper.c)
add_library(testExePluginHelperObj OBJECT ${src}/testExePluginHelperObj.c)
set_property(TARGET testExeWithPluginHelper PROPERTY ENABLE_EXPORTS 1)
set_property(TARGET testExeWithPluginHelper PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT $<TARGET_NAME:testExePluginHelperObj>)
set_property(TARGET testExeWithPluginHelper PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE $<1:testExePluginHelperExclude>)
