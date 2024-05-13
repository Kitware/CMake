include(${CMAKE_ROOT}/Modules/Internal/CMakeDetermineLinkerId.cmake)

set(tools
  aix7.3-ld
  debian12-ld.bfd
  debian12-ld.gold
  debian12-ld.lld
  debian12-ld.mold
  fedora39-ld.bfd
  fedora39-ld.gold
  fedora39-ld.lld
  fedora39-ld.mold
  msvc14.36-link
  sunos5.11-ld
  xcode15.1-ld
  )

foreach(tool IN LISTS tools)
  block()
    include(${CMAKE_CURRENT_LIST_DIR}/ld-v/${tool}.cmake OPTIONAL)
    cmake_determine_linker_id(C ${CMAKE_CURRENT_LIST_DIR}/ld-v/${tool}.bash)
    file(STRINGS ${CMAKE_CURRENT_LIST_DIR}/ld-v/${tool}.txt results)
    foreach(result IN LISTS results)
      if(result MATCHES "^([A-Z_]+)='([^']*)'")
        set(expect_var "${CMAKE_MATCH_1}")
        set(expect_val "${CMAKE_MATCH_2}")
        if(NOT "x${${expect_var}}" STREQUAL "x${expect_val}")
          message(SEND_ERROR "${tool} result\n"
            "  ${expect_var}='${${expect_var}}'\n"
            "is not expected\n"
            "  ${expect_var}='${expect_val}'\n"
            )
        endif()
      endif()
    endforeach()
  endblock()
endforeach()
