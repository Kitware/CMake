include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

string(JSON document GET "${content}" @graph 1)
expect_array("${document}" 1 rootElement)
expect_array("${document}" 4 element)
expect_array("${content}" 12 @graph)

set(targets root middle leaf side tail)
set(licenses MIT Apache-2.0 BSD-3-Clause Zlib ISC)
foreach(target license IN ZIP_LISTS targets licenses)
  set(package_expected "{\"spdxId\":\"urn:${target}#Package\"}")
  if(target STREQUAL "root")
    expect_object("${document}" package_expected rootElement)
  else()
    expect_object("${document}" package_expected element)
  endif()

  set(license_expected "{
    \"from\": \"urn:${target}#Package\",
    \"relationshipType\": \"hasDeclaredLicense\",
    \"to\": [{\"simplelicensing_licenseExpression\": \"${license}\"}]
  }")
  expect_object("${content}" license_expected @graph)
endforeach()

set(dependencies_root middle side)
set(dependencies_middle leaf)
set(dependencies_side leaf)
set(dependencies_leaf tail)
set(dependencies_tail middle)
foreach(target IN LISTS targets)
  set(to)
  foreach(dependency IN LISTS dependencies_${target})
    list(APPEND to "\"urn:${dependency}#Package\"")
  endforeach()
  list(JOIN to ", " to)
  set(links_expected "{
    \"from\": \"urn:${target}#Package\",
    \"relationshipType\": \"dependsOn\",
    \"to\": [${to}]
  }")
  expect_object("${content}" links_expected @graph)
endforeach()
