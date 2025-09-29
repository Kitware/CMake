enable_language(C)

add_library(foo STATIC LinkLikewiseLib.c)

add_executable(main1 LinkLikewiseMain.c)
add_executable(main2 LinkLikewiseMain.c)

# main1 depends on foo.
target_link_libraries(main1 PRIVATE foo)

# main2 depends on foo, but express it by referencing main1's dependency.
target_link_libraries(main2 PRIVATE "$<TARGET_PROPERTY:main1,LINK_LIBRARIES>")
