find_package(
    bar 1.3.4 REQUIRED CONFIG
    NO_DEFAULT_PATH
    PATHS ${CMAKE_CURRENT_LIST_DIR}/config
)

add_library(foo INTERFACE)
target_link_libraries(foo INTERFACE bar::bar)

install(TARGETS foo EXPORT foo DESTINATION .)
export(PACKAGE_INFO foo EXPORT foo)
