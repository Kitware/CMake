# Logic common to InterfaceLinkLibrariesDirect and ExportImport tests.
set(src ${CMAKE_CURRENT_LIST_DIR})
add_library(testSharedLibWithHelper SHARED ${src}/testSharedLibWithHelper.c)
add_library(testSharedLibHelperObj OBJECT ${src}/testSharedLibHelperObj.c)
set_property(TARGET testSharedLibWithHelper PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT $<TARGET_NAME:testSharedLibHelperObj>)
set_property(TARGET testSharedLibWithHelper PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE $<1:testSharedLibHelperExclude>)
