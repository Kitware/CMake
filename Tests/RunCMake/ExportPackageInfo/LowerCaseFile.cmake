add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)
export(PACKAGE_INFO LowerCase EXPORT foo LOWER_CASE_FILE)
export(PACKAGE_INFO PreserveCase EXPORT foo)
