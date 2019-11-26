enable_language(CXX)

add_subdirectory(ExcludeFromAll EXCLUDE_FROM_ALL)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE foo)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake CONTENT "
set(main_exe \"$<TARGET_FILE:main>\")
set(foo_lib \"$<TARGET_FILE:foo>\")
set(bar_lib \"$<TARGET_FILE:bar>\")
set(zot_lib \"$<TARGET_FILE:zot>\")
set(subinc_lib \"$<TARGET_FILE:subinc>\")
set(subsub_lib \"$<TARGET_FILE:subsub>\")
set(subsubinc_lib \"$<TARGET_FILE:subsubinc>\")
")
