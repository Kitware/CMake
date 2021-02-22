function(xcode_sign_adhoc target)
  if(CMAKE_GENERATOR STREQUAL "Xcode" AND
     "${CMAKE_SYSTEM_NAME};${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "Darwin;arm64")
    # Xcode runs POST_BUILD before signing, so let the linker use ad-hoc signing.
    # See CMake Issue 21845.
    target_link_options(${target} PRIVATE LINKER:-adhoc_codesign)
  endif()
endfunction()
