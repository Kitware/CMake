cmake_policy(SET CMP0189 NEW)
include(LinkLikewise-common.cmake)

# Test that CMP0189 NEW tolerates circular LINK_LIBRARIES references.
target_link_libraries(main1 PRIVATE "$<TARGET_PROPERTY:main2,LINK_LIBRARIES>")
