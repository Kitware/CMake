include(BundleUtilities)

set(BU_CHMOD_BUNDLE_ITEMS ON)

function(check_script script)
  fixup_bundle_item(${script} ${script} "" "")
endfunction()

# Should not throw any errors
# Shell script
set(script_sh_EMBEDDED_ITEM ${CMAKE_CURRENT_LIST_DIR}/test.app/script.sh)
check_script(${CMAKE_CURRENT_LIST_DIR}/test.app/script.sh)
# Batch script
set(script_bat_EMBEDDED_ITEM ${CMAKE_CURRENT_LIST_DIR}/test.app/script.bat)
check_script(${CMAKE_CURRENT_LIST_DIR}/test.app/script.bat)
# Shell script without extension
set(script_EMBEDDED_ITEM ${CMAKE_CURRENT_LIST_DIR}/test.app/script)
check_script(${CMAKE_CURRENT_LIST_DIR}/test.app/script)
