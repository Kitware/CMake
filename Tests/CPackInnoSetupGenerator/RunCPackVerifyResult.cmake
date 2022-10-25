message(STATUS "=============================================================")
message(STATUS "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")
message(STATUS "")

if(NOT CPackInnoSetupGenerator_BINARY_DIR)
  message(FATAL_ERROR "CPackInnoSetupGenerator_BINARY_DIR not set")
endif()

message(STATUS "CMAKE_COMMAND: ${CMAKE_COMMAND}")
message(STATUS "CMAKE_CPACK_COMMAND: ${CMAKE_CPACK_COMMAND}")
message(STATUS "CPackInnoSetupGenerator_BINARY_DIR: ${CPackInnoSetupGenerator_BINARY_DIR}")

if(config)
  set(_C_config -C ${config})
endif()

execute_process(COMMAND "${CMAKE_CPACK_COMMAND}"
  ${_C_config}
  RESULT_VARIABLE CPack_result
  OUTPUT_VARIABLE CPack_output
  ERROR_VARIABLE CPack_output
  WORKING_DIRECTORY "${CPackInnoSetupGenerator_BINARY_DIR}")

if(CPack_result)
  message(FATAL_ERROR "CPack execution went wrong!, Output: ${CPack_output}")
else ()
  message(STATUS "Output: ${CPack_output}")
endif()

file(GLOB project_file "${CPackInnoSetupGenerator_BINARY_DIR}/_CPack_Packages/win32/INNOSETUP/ISScript.iss")
file(GLOB installer_file "${CPackInnoSetupGenerator_BINARY_DIR}/_CPack_Packages/win32/INNOSETUP/hello_world_setup.exe")

message(STATUS "Project file: '${project_file}'")
message(STATUS "Installer file: '${installer_file}'")

if(NOT project_file)
  message(FATAL_ERROR "Project file does not exist")
endif()

if(NOT installer_file)
  message(FATAL_ERROR "Installer file does not exist")
endif()

# Test if the correct registry key is set
file(STRINGS "${project_file}" results REGEX "^AppId=hello_world$")
if(results STREQUAL "")
  message(FATAL_ERROR "CPACK_PACKAGE_INSTALL_REGISTRY_KEY doesn't match AppId")
endif()

# Test if only readme page is shown
file(STRINGS "${project_file}" results REGEX "^LicenseFile=")
file(STRINGS "${project_file}" results2 REGEX "^InfoBeforeFile=")
if(NOT results STREQUAL "" OR results2 STREQUAL "")
  message(FATAL_ERROR "Erroneous output with license and readme files")
endif()

# Test if classic style is used by default
file(STRINGS "${project_file}" results REGEX "compiler:SetupClassicIcon\\.ico")
file(STRINGS "${project_file}" results2 REGEX "compiler:WizClassicImage\\.bmp")
if(results STREQUAL "" OR results2 STREQUAL "")
  message(FATAL_ERROR "Images of classic style not used")
endif()

# Test if the top-level start menu folder is used
file(STRINGS "${project_file}" results REGEX "{autoprograms}")
file(STRINGS "${project_file}" results2 REGEX "{group}")
if(results STREQUAL "" OR NOT results2 STREQUAL "")
  message(FATAL_ERROR "Top-level start menu folder not used")
endif()

# Test CPACK_INNOSETUP_USE_CMAKE_BOOL_FORMAT
file(STRINGS "${project_file}" results REGEX "^AppComments=yes$")
if(results STREQUAL "")
  message(FATAL_ERROR "CPACK_INNOSETUP_USE_CMAKE_BOOL_FORMAT doesn't convert booleans")
endif()

# Test the custom installation rule
file(STRINGS "${project_file}" results REGEX "^Name: \"{userdocs}\\\\empty\"; Check: ReturnTrue; Components: accessories\\\\extras$")
if(results STREQUAL "")
  message(FATAL_ERROR "Custom installation rule not found or incomplete")
endif()

# Test if an uninstall shortcut has been created
file(STRINGS "${project_file}" results REGEX "{uninstallexe}")
if(results STREQUAL "")
  message(FATAL_ERROR "No uninstall shortcut created")
endif()

# Test CPACK_INNOSETUP_<compName>_INSTALL_DIRECTORY
file(STRINGS "${project_file}" results REGEX "{app}.*Components: accessories\\\\extras")
if(NOT results STREQUAL "")
  message(FATAL_ERROR "Component not in custom install directory")
endif()

# Test if component names are nested correctly
file(STRINGS "${project_file}" results REGEX "Components:.* extras")
if(NOT results STREQUAL "")
  message(FATAL_ERROR "Component names must contain their parent groups according to the documentation")
endif()

# Test if custom installation type exists
file(STRINGS "${project_file}" results REGEX "Flags: .*iscustom")
if(results STREQUAL "")
  message(FATAL_ERROR "Custom installation type doesn't exist")
endif()

# Test if hidden components are processed but not displayed
file(STRINGS "${project_file}" results REGEX "Source:.+hidden_component\\\\my_file\\.txt")
file(STRINGS "${project_file}" results2 REGEX "Name: \"hidden_component\"")
if(results STREQUAL "" OR NOT results2 STREQUAL "")
  message(FATAL_ERROR "Hidden component displayed or one of its files ignored")
endif()

# Test if disabled and hidden components are ignored at all
file(STRINGS "${project_file}" results REGEX "Source:.+hidden_component2\\\\my_file\\.txt")
if(NOT results STREQUAL "")
  message(FATAL_ERROR "Disabled and hidden component not ignored")
endif()

# Test if required components ignore their installation types
file(STRINGS "${project_file}" results REGEX "Types: (basic|full|custom|basic full|full basic|basic custom|full custom); Flags: fixed")
if(NOT results STREQUAL "")
  message(FATAL_ERROR "Required components don't ignore their installation types")
endif()

# Test constant escape (should contain Hello%2c World!)
file(STRINGS "${project_file}" results REGEX "Hello%2c World!")
if(results STREQUAL "")
  message(FATAL_ERROR "The comma character isn't escaped to %2c")
endif()

# Test double quote syntax
file(STRINGS "${project_file}" results REGEX "\"\"Large\"\"")
if(results STREQUAL "")
  message(FATAL_ERROR "The quote character isn't escaped correctly")
endif()
