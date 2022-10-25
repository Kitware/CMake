cmake_minimum_required(VERSION 3.13)

project(CPackInnoSetupGenerator VERSION 42.0 HOMEPAGE_URL "https://www.example.com")

add_executable(hello main.c)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/empty)

install(TARGETS hello DESTINATION / COMPONENT application)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/empty DESTINATION / COMPONENT extras)
install(FILES my_bitmap.bmp DESTINATION awesome COMPONENT extras)
install(FILES my_file.txt DESTINATION / COMPONENT hidden_component)
install(FILES my_file.txt DESTINATION / COMPONENT hidden_component2)

set(CPACK_GENERATOR "INNOSETUP")

set(CPACK_PACKAGE_NAME "Hello, World!") # Test constant escape (like {cm:...}, see code documentation)
set(CPACK_PACKAGE_VENDOR "Sheldon Cooper")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "hello_world")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "hello_world")
set(CPACK_PACKAGE_FILE_NAME "hello_world_setup")
set(CPACK_SYSTEM_NAME "win32")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/my_bitmap.bmp")
set(CPACK_VERBATIM_VARIABLES ON)
set(CPACK_PACKAGE_EXECUTABLES "hello" "Hello, World!")
set(CPACK_CREATE_DESKTOP_LINKS hello)

set(CPACK_INNOSETUP_INSTALL_ROOT "{autopf}\\Sheldon Cooper")
set(CPACK_INNOSETUP_PROGRAM_MENU_FOLDER ".")
set(CPACK_INNOSETUP_IGNORE_LICENSE_PAGE ON)
set(CPACK_INNOSETUP_IGNORE_README_PAGE OFF) # Test if only readme page is shown
set(CPACK_INNOSETUP_SETUP_AppComments ON) # Test if CPACK_INNOSETUP_USE_CMAKE_BOOL_FORMAT works
set(CPACK_INNOSETUP_CUSTOM_INSTALL_INSTRUCTIONS "extras/empty"
    "Name: \"{userdocs}\\empty\"\; Check: ReturnTrue\; Components: accessories\\extras")
set(CPACK_INNOSETUP_MENU_LINKS "https://www.example.com" "Web"
    "my_file.txt" "Text")
set(CPACK_INNOSETUP_RUN_EXECUTABLES hello)
set(CPACK_INNOSETUP_CREATE_UNINSTALL_LINK ON)
# Test if this macro is available in the code file below containing the check function
set(CPACK_INNOSETUP_DEFINE_PascalMacro "end;")
set(CPACK_INNOSETUP_CODE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Code.pas")
set(CPACK_INNOSETUP_EXECUTABLE "ISCC.exe")

include(CPackComponent)

cpack_add_install_type(basic DISPLAY_NAME "Basic installation")
cpack_add_install_type(full DISPLAY_NAME "\"Large\" installation") # Test double quote syntax
cpack_add_component_group(accessories DISPLAY_NAME "Accessories")

cpack_add_component(application DISPLAY_NAME "Application" INSTALL_TYPES basic full REQUIRED)
cpack_add_component(extras DISPLAY_NAME "Additional components" INSTALL_TYPES full GROUP accessories)
cpack_add_component(hidden_component HIDDEN)
cpack_add_component(hidden_component2 HIDDEN DISABLED)
set(CPACK_INNOSETUP_extras_INSTALL_DIRECTORY "{userdocs}")

include(CPack)
