if(MSVC)
  if(NOT CMAKE_VERSION VERSION_LESS 3.15)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  else()
    foreach(CONFIG DEBUG MINSIZEREL RELEASE RELWITHDEBINFO)
      string(REPLACE "/MD" "/MT"
        "CMAKE_CXX_FLAGS_${CONFIG}"
        "${CMAKE_CXX_FLAGS_${CONFIG}}"
        )
      string(REPLACE "-MD" "-MT"
        "CMAKE_CXX_FLAGS_${CONFIG}"
        "${CMAKE_CXX_FLAGS_${CONFIG}}"
        )
    endforeach()
  endif()
endif()

add_library(CMakeWiXCustomActions MODULE
  detect_nsis_overwrite.cpp
  exports.def
)

target_link_libraries(CMakeWiXCustomActions PRIVATE msi)
