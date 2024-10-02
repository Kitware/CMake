enable_language(CXX)

set_property(SOURCE "foo.cpp" PROPERTY VS_TOOL_OVERRIDE CustomFooCppTool)
set_property(SOURCE "foo.txt" PROPERTY VS_TOOL_OVERRIDE CustomFooTxtTool)
add_library(foo foo.cpp foo.txt)

add_library(bar bar.cpp bar.txt)
