add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)
export(EXPORT foo PACKAGE_INFO LowerCase LOWER_CASE_FILE)
export(EXPORT foo PACKAGE_INFO PreserveCase)
