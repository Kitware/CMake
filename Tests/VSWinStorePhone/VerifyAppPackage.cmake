set(APP_PKG_NAME Direct3DApp1)

# List of files that are expected to be present in the generated app package
set(EXPECTED_APP_PKG_CONTENT
  ${APP_PKG_NAME}.exe
  CxxDll.dll
  JusticeLeagueWinRT.winmd
  JusticeLeagueWinRT.dll
)

# Windows app package formats can be either msix, appx or xap
file(GLOB_RECURSE ALL_APP_PKG_FILES ${APP_PACKAGE_DIR} ${APP_PKG_NAME}*.msix ${APP_PKG_NAME}*.appx ${APP_PKG_NAME}*.xap)

# There can be only one generated app package
list(LENGTH ALL_APP_PKG_FILES APP_PKG_COUNT)
if(NOT APP_PKG_COUNT EQUAL 1)
  message(FATAL_ERROR "Expected 1 generated app package, but detected ${APP_PKG_COUNT}: ${ALL_APP_PKG_FILES}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E tar tf ${ALL_APP_PKG_FILES}
  OUTPUT_VARIABLE APP_PKG_CONTENT_OUTPUT
  ERROR_VARIABLE error
  RESULT_VARIABLE result)

if(NOT result EQUAL 0)
  message(FATAL_ERROR "Listing app package content failed with: ${error}")
endif()

foreach(app_pkg_item IN LISTS EXPECTED_APP_PKG_CONTENT)
  string(FIND ${APP_PKG_CONTENT_OUTPUT} ${app_pkg_item} _found)
  if(_found EQUAL -1)
    message(FATAL_ERROR "Generated app package is missing an expected item: ${app_pkg_item}")
  endif()
endforeach()
