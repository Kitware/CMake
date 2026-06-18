# Collect "path -> object id" for every product PBXFileReference (those under
# BUILT_PRODUCTS_DIR) in a generated project.pbxproj.
function(read_product_ids pbxproj out_var)
  if(NOT EXISTS "${pbxproj}")
    set(RunCMake_TEST_FAILED "Project file does not exist:\n  ${pbxproj}" PARENT_SCOPE)
    return()
  endif()
  set(ids "")
  file(STRINGS "${pbxproj}" lines)
  foreach(line IN LISTS lines)
    if(line MATCHES "isa = PBXFileReference;" AND line MATCHES "sourceTree = BUILT_PRODUCTS_DIR;")
      string(REGEX MATCH "^[ \t]*([0-9A-F]+) " _ "${line}")
      set(id "${CMAKE_MATCH_1}")
      string(REGEX MATCH "path = ([^;]+);" _ "${line}")
      set(path "${CMAKE_MATCH_1}")
      list(APPEND ids "${path}=${id}")
    endif()
  endforeach()
  list(SORT ids)
  set(${out_var} "${ids}" PARENT_SCOPE)
endfunction()

set(pbxproj "${RunCMake_TEST_BINARY_DIR}/DeterministicProductIds.xcodeproj/project.pbxproj")
read_product_ids("${pbxproj}" this_ids)
if(RunCMake_TEST_FAILED)
  return()
endif()

if(NOT this_ids)
  set(RunCMake_TEST_FAILED "No product PBXFileReference entries found in\n  ${pbxproj}")
  return()
endif()

# On the second generation, compare against the first build tree. The ids must
# be byte-identical even though the build tree path differs.
if(DEFINED DeterministicProductIds_FirstBinaryDir)
  read_product_ids(
    "${DeterministicProductIds_FirstBinaryDir}/DeterministicProductIds.xcodeproj/project.pbxproj"
    first_ids)
  if(RunCMake_TEST_FAILED)
    return()
  endif()
  if(NOT this_ids STREQUAL first_ids)
    string(REPLACE ";" "\n  " a "${first_ids}")
    string(REPLACE ";" "\n  " b "${this_ids}")
    set(RunCMake_TEST_FAILED
      "Product PBXFileReference ids are not deterministic across build trees.\nFirst:\n  ${a}\nSecond:\n  ${b}")
  endif()
endif()
