enable_language(${language})

function(sharedLibraryDefs_addTests policy_value)
  block(SCOPE_FOR POLICIES)
    if (NOT "${policy_value}" STREQUAL "WARN")
      # WARN is the default value - no need to set it
      cmake_policy(SET CMP0203 "${policy_value}")
    endif()

    add_executable(${language}-CMP0203_${policy_value} checker.${extension})
    target_compile_definitions(${language}-CMP0203_${policy_value} PRIVATE "FUNCTION=main")

    add_library(${language}-CMP0203_${policy_value}_shared SHARED checker.${extension})
    target_compile_definitions(${language}-CMP0203_${policy_value}_shared PRIVATE "FUNCTION=test")

    add_library(${language}-CMP0203_${policy_value}_static STATIC checker.${extension})
    target_compile_definitions(${language}-CMP0203_${policy_value}_static PRIVATE "FUNCTION=test")
  endblock()
endfunction()

sharedLibraryDefs_addTests(OLD)
sharedLibraryDefs_addTests(WARN)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
  # Visual Studio always defines `_WINDLL`
  target_compile_definitions(${language}-CMP0203_OLD_shared PRIVATE "MUST_HAVE_DEFINE")
  target_compile_definitions(${language}-CMP0203_WARN_shared PRIVATE "MUST_HAVE_DEFINE")
endif()

sharedLibraryDefs_addTests(NEW)

if ("${CMAKE_${language}_COMPILER_ID}" STREQUAL "MSVC" OR "${CMAKE_${language}_SIMULATE_ID}" STREQUAL "MSVC")
  target_compile_definitions(${language}-CMP0203_NEW_shared PRIVATE "MUST_HAVE_DEFINE")
endif()
