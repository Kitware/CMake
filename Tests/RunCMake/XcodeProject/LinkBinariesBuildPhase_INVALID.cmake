enable_language(CXX)

add_executable(app main.cpp)
set_target_properties(app PROPERTIES XCODE_LINK_BUILD_PHASE_MODE INVALID)
