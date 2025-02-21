enable_language(CXX)
add_executable(app MACOSX_BUNDLE app.cxx)
add_library(fw SHARED fw.cxx)
set_property(TARGET fw PROPERTY FRAMEWORK 1)
foreach(target IN ITEMS app fw)
  set_property(TARGET ${target} PROPERTY XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT dwarf-with-dsym)
  add_custom_command(
    TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E sha256sum
    [["$DWARF_DSYM_FOLDER_PATH/$DWARF_DSYM_FILE_NAME/Contents/Resources/DWARF/$PRODUCT_NAME"]]
    [["$DWARF_DSYM_FOLDER_PATH/$DWARF_DSYM_FILE_NAME/Contents/Info.plist"]]
    [["$TARGET_BUILD_DIR/$EXECUTABLE_PATH"]]
    [["$TARGET_BUILD_DIR/$INFOPLIST_PATH"]]
  )
endforeach()
