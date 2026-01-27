enable_language(${language})

function(msvcCharsetDefs_addTests policy_value suffix expected_define passed_define)
  block(SCOPE_FOR POLICIES)
    if (NOT "${policy_value}" STREQUAL "WARN")
      # WARN is the default value - no need to set it
      cmake_policy(SET CMP0204 "${policy_value}")
    endif()

    set(suffix "CMP0204-${policy_value}_${suffix}")

    add_executable(${language}-${suffix} checker.${extension})
    target_compile_definitions(${language}-${suffix} PRIVATE
      "MUST_HAVE_DEFINE_${expected_define}"
      "${passed_define}"
    )
  endblock()
endfunction()

foreach(policy_value IN ITEMS WARN NEW)
  msvcCharsetDefs_addTests(${policy_value} MBCS_NO_VALUE MBCS _MBCS)
  msvcCharsetDefs_addTests(${policy_value} SBCS_NO_VALUE SBCS _SBCS)
  msvcCharsetDefs_addTests(${policy_value} UNICODE_NO_VALUE UNICODE _UNICODE)
  msvcCharsetDefs_addTests(${policy_value} MBCS_WITH_VALUE MBCS _MBCS=1)
  msvcCharsetDefs_addTests(${policy_value} SBCS_WITH_VALUE SBCS _SBCS=1)
  msvcCharsetDefs_addTests(${policy_value} UNICODE_WITH_VALUE UNICODE _UNICODE=1)

  set(expected_define "")
  if (CMAKE_GENERATOR MATCHES "Visual Studio")
    # Visual Studio always defines `_MBCS` by default
    set(expected_define "MBCS")
  elseif (policy_value STREQUAL "NEW" AND
    ("${CMAKE_${language}_COMPILER_ID}" STREQUAL "MSVC" OR "${CMAKE_${language}_SIMULATE_ID}" STREQUAL "MSVC"))
    set(expected_define "MBCS")
  endif()
  msvcCharsetDefs_addTests(${policy_value} Default "${expected_define}" "")
endforeach()
