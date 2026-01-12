find_package(
    baz 1.3.4 REQUIRED
    NO_DEFAULT_PATH
    PATHS ${CMAKE_CURRENT_LIST_DIR}
)

add_library(foo INTERFACE)
target_link_libraries(foo INTERFACE baz::baz)

install(TARGETS foo EXPORT foo DESTINATION .)
export(PACKAGE_INFO foo EXPORT foo)
