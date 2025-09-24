enable_language(${language})

function(msvcCharsetDefs_addTests policy_value suffix expected_define passed_define)
  block(SCOPE_FOR POLICIES)
    if (NOT "${policy_value}" STREQUAL "WARN")
      # WARN is the default value - no need to set it
      cmake_policy(SET CMP0204 "${policy_value}")
    endif()

    set(suffix "CMP0204-${policy_value}_${suffix}")

    add_executable(${language}-${suffix}_executable checker.${extension})
    target_compile_definitions(${language}-${suffix}_executable PRIVATE "FUNCTION=main")

    add_library(${language}-${suffix}_shared SHARED checker.${extension})
    target_compile_definitions(${language}-${suffix}_shared PRIVATE "FUNCTION=test")

    add_library(${language}-${suffix}_static STATIC checker.${extension})
    target_compile_definitions(${language}-${suffix}_static PRIVATE "FUNCTION=test")

    add_library(${language}-${suffix}_object OBJECT checker.${extension})
    target_compile_definitions(${language}-${suffix}_object PRIVATE "FUNCTION=test")

    foreach(target IN ITEMS ${language}-${suffix}_executable ${language}-${suffix}_shared ${language}-${suffix}_static ${language}-${suffix}_object)
      target_compile_definitions(${target} PRIVATE
        "MUST_HAVE_DEFINE_${expected_define}"
        "${passed_define}"
      )
    endforeach()
  endblock()
endfunction()

foreach(policy_value IN ITEMS OLD WARN NEW)
  msvcCharsetDefs_addTests(${policy_value} MBCS_NO_VALUE MBCS _MBCS)
  msvcCharsetDefs_addTests(${policy_value} SBCS_NO_VALUE SBCS _SBCS)
  msvcCharsetDefs_addTests(${policy_value} UNICODE_NO_VALUE UNICODE _UNICODE)
  msvcCharsetDefs_addTests(${policy_value} MBCS_WITH_VALUE MBCS _MBCS=1)
  msvcCharsetDefs_addTests(${policy_value} SBCS_WITH_VALUE SBCS _SBCS=1)
  msvcCharsetDefs_addTests(${policy_value} UNICODE_WITH_VALUE UNICODE _UNICODE=1)
  msvcCharsetDefs_addTests(${policy_value} D_MBCS_NO_VALUE MBCS -D_MBCS)
  msvcCharsetDefs_addTests(${policy_value} D_SBCS_NO_VALUE SBCS -D_SBCS)
  msvcCharsetDefs_addTests(${policy_value} D_UNICODE_NO_VALUE UNICODE -D_UNICODE)
  msvcCharsetDefs_addTests(${policy_value} D_MBCS_WITH_VALUE MBCS -D_MBCS=1)
  msvcCharsetDefs_addTests(${policy_value} D_SBCS_WITH_VALUE SBCS -D_SBCS=1)
  msvcCharsetDefs_addTests(${policy_value} D_UNICODE_WITH_VALUE UNICODE -D_UNICODE=1)

  set(expected_define "")
  if (CMAKE_GENERATOR MATCHES "Visual Studio")
    # Visual Studio always defines `_MBCS` by default
    set(expected_define "MBCS")
  elseif (policy_value STREQUAL "NEW")
    set(expected_define "MBCS")
  endif()
  msvcCharsetDefs_addTests(${policy_value} Default "${expected_define}" "")
endforeach()
