
add_library(visibility_preset SHARED lib.cpp)
set_property(TARGET visibility_preset PROPERTY CXX_VISIBILITY_PRESET hiden)
