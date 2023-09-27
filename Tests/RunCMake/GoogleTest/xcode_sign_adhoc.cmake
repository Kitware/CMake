function(xcode_sign_adhoc target)
  if(CMAKE_GENERATOR STREQUAL "Xcode" AND
      "${CMAKE_SYSTEM_NAME};${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "Darwin;arm64")
    if(XCODE_VERSION VERSION_GREATER_EQUAL 15)
      # Xcode 15+ enforces '-Xlinker -no_adhoc_codesign' after user flags,
      # so we cannot convince the linker to add an adhoc signature.
      add_custom_command(TARGET ${target} POST_BUILD COMMAND codesign --sign - --force "$<TARGET_FILE:${target}>")
    else()
      # Xcode runs POST_BUILD before signing, so let the linker use ad-hoc signing.
      # See CMake Issue 21845.
      target_link_options(${target} PRIVATE LINKER:-adhoc_codesign)
    endif()
  endif()
endfunction()
