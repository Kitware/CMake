enable_language(C)
add_executable(main main.c)
add_custom_target(custom1)
add_custom_target(custom2)
file(WRITE "${CMAKE_BINARY_DIR}/XcodeWorkspace.xcworkspace/contents.xcworkspacedata" [[
<?xml version="1.0" encoding="UTF-8"?>
<Workspace version = "1.0">
   <FileRef location = "container:XcodeWorkspace.xcodeproj"/>
</Workspace>
]])
