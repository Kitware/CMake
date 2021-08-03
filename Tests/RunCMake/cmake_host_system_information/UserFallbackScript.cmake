list(
    APPEND CMAKE_GET_OS_RELEASE_FALLBACK_SCRIPTS
    ${CMAKE_CURRENT_SOURCE_DIR}/000-FirstFallbackScript.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/Ignored-Script.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/999-LastFallbackScript.cmake
  )

cmake_host_system_information(RESULT UFS QUERY DISTRIB_INFO)

foreach(VAR IN LISTS UFS)
  message(STATUS "${VAR}=`${${VAR}}`")
endforeach()
