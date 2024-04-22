set(expect [[
# file\(GENERATE\) produced:
main LINK_LIBRARIES: 'foo1' # not transitive
main LINK_DIRECTORIES: '[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/dirM;[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/dir1;[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/dir2'
main LINK_OPTIONS: '-optM;-opt1;-opt2'
main LINK_DEPENDS: '[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/TransitiveLink-CMP0166-NEW-build/depM;[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/TransitiveLink-CMP0166-NEW-build/dep1;[^';]*/Tests/RunCMake/GenEx-TARGET_PROPERTY/TransitiveLink-CMP0166-NEW-build/dep2'
]])
include(${CMAKE_CURRENT_LIST_DIR}/TransitiveLink-check-common.cmake)
