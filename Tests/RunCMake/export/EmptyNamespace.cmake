add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo)

# Ensure we can export with an explicitly-empty namespace.
export(EXPORT foo NAMESPACE "")
