enable_language(C)

# A module library defaults to the runtime destination on DLL platforms and to
# the library destination elsewhere, but it is always a LIBRARY artifact:
# LIBRARY DESTINATION overrides the default on every platform, and RUNTIME
# DESTINATION never applies to it.
add_library(mod1 MODULE obj1.c)
add_library(mod2 MODULE obj2.c)
add_library(mod3 MODULE obj3.c)

install(TARGETS mod1)
install(TARGETS mod2 LIBRARY DESTINATION plugins)
install(TARGETS mod3 RUNTIME DESTINATION notused)
