function(CheckMatch actual expect)
  string(STRIP "${actual}" trimmed)
  if(NOT "${trimmed}" MATCHES "${expect}")
    message(SEND_ERROR "The actual project contains the line:\n ${trimmed}\n"
      "which does not match expected regex:\n ${expect}\n")
  endif()
endfunction()

set(expect "developmentRegion = de")
file(STRINGS ${RunCMake_TEST_BINARY_DIR}/XcodeAttributeRegions.xcodeproj/project.pbxproj actual
  REGEX "${expect}" LIMIT_COUNT 1)
CheckMatch("${actual}" "${expect}")

set(expect "knownRegions = \\(")
file(STRINGS ${RunCMake_TEST_BINARY_DIR}/XcodeAttributeRegions.xcodeproj/project.pbxproj actual
  REGEX "knownRegions = \\(" LIMIT_COUNT 1)
CheckMatch("${actual}" "${expect}")

set(expect "en,de,uk\t\t\t\\)")
file(STRINGS ${RunCMake_TEST_BINARY_DIR}/XcodeAttributeRegions.xcodeproj/project.pbxproj actual
  REGEX "${expect}" LIMIT_COUNT 1)
CheckMatch("${actual}" "${expect}")
