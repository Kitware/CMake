enable_language(C)
find_package(gui REQUIRED)

add_library(baz INTERFACE)

target_link_libraries(baz INTERFACE gui::gui gui::widget)
