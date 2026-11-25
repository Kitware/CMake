set(expect-tool "^compiled\\.mach-o\\.executable$")
set(expect-app "^wrapper\\.application$")
set(expect-app_ext_app "^wrapper\\.application$")
set(expect-app_clip "^wrapper\\.application$")
set(expect-explicit "^wrapper\\.app-extension$")

set(pbxproj "${RunCMake_TEST_BINARY_DIR}/XcodeProductFileType.xcodeproj/project.pbxproj")

function(get_product_file_type target out_var)
  file(STRINGS "${pbxproj}" references
    REGEX "isa = PBXFileReference;.* path = \"?${target}\"?; sourceTree = BUILT_PRODUCTS_DIR;")
  set(file_type "")
  # file(STRINGS) escapes semicolons, so iterate to get the original line.
  foreach(line IN LISTS references)
    if(line MATCHES "explicitFileType = \"?([^\";]+)\"?;")
      set(file_type "${CMAKE_MATCH_1}")
    endif()
  endforeach()
  set(${out_var} "${file_type}" PARENT_SCOPE)
endfunction()

foreach(target IN ITEMS tool app app_ext_app app_clip explicit)
  get_product_file_type(${target} actual)
  if(NOT actual MATCHES "${expect-${target}}")
    string(APPEND RunCMake_TEST_FAILED
      "Product file type of '${target}' is '${actual}', expected '${expect-${target}}'\n")
  endif()
endforeach()

# Bundles that are not applications must not be treated as such.
foreach(target IN ITEMS xpc_by_extension xpc_by_product_type app_extension)
  get_product_file_type(${target} actual)
  if(actual STREQUAL "" OR actual STREQUAL "wrapper.application")
    string(APPEND RunCMake_TEST_FAILED
      "Product file type of '${target}' is '${actual}', expected non-application type\n")
  endif()
endforeach()
