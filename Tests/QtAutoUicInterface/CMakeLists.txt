
cmake_minimum_required(VERSION 2.8.12)

project(QtAutoUicInterface)

find_package(Qt5Widgets 5.3 REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# BEGIN Upstream

add_library(KI18n klocalizedstring.cpp)
target_link_libraries(KI18n Qt5::Core)

set(autouic_options
  -include klocalizedstring.h
  -tr tr2$<$<NOT:$<BOOL:$<TARGET_PROPERTY:NO_KUIT_SEMANTIC>>>:x>i18n
)
set_property(TARGET KI18n APPEND PROPERTY
  INTERFACE_AUTOUIC_OPTIONS ${autouic_options}
)

set(domainProp $<TARGET_PROPERTY:TRANSLATION_DOMAIN>)
set(nameLower $<LOWER_CASE:$<MAKE_C_IDENTIFIER:$<TARGET_PROPERTY:NAME>>>)
set(domain_logic
  $<$<BOOL:${domainProp}>:${domainProp}>$<$<NOT:$<BOOL:${domainProp}>>:${nameLower}>
)
set_property(TARGET KI18n APPEND PROPERTY
  INTERFACE_COMPILE_DEFINITIONS "TRANSLATION_DOMAIN=${domain_logic}"
)

# END upstream

add_library(LibWidget libwidget.cpp)
target_link_libraries(LibWidget KI18n Qt5::Widgets)
set_property(TARGET LibWidget PROPERTY NO_KUIT_SEMANTIC ON)
set_property(TARGET LibWidget PROPERTY TRANSLATION_DOMAIN customdomain)

add_library(MyWidget mywidget.cpp)
target_link_libraries(MyWidget KI18n Qt5::Widgets)

add_executable(QtAutoUicInterface main.cpp)
target_compile_definitions(QtAutoUicInterface
  PRIVATE
    UI_LIBWIDGET_H="${CMAKE_CURRENT_BINARY_DIR}/ui_libwidget.h"
    UI_MYWIDGET_H="${CMAKE_CURRENT_BINARY_DIR}/ui_mywidget.h"
)
