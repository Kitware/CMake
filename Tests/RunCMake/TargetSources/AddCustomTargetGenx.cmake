add_custom_target(target ALL)
target_sources(target PRIVATE $<IF:1,${CMAKE_CURRENT_LIST_DIR}/main.cpp,${CMAKE_CURRENT_LIST_DIR}/empty_1.cpp>)
