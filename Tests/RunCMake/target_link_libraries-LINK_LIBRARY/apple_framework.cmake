
enable_language(OBJCXX)


# feature FRAMEWORK
add_library(foo-framework SHARED foo.mm)
target_link_libraries(foo-framework PRIVATE "$<LINK_LIBRARY:FRAMEWORK,Foundation>")

add_executable(main-framework main.mm)
target_link_libraries(main-framework PRIVATE "$<LINK_LIBRARY:FRAMEWORK,Foundation>" foo-framework)


# feature NEEDED_FRAMEWORK
add_library(foo-needed_framework SHARED foo.mm)
target_link_libraries(foo-needed_framework PRIVATE "$<LINK_LIBRARY:NEEDED_FRAMEWORK,Foundation>")

add_executable(main-needed_framework main.mm)
target_link_libraries(main-needed_framework PRIVATE "$<LINK_LIBRARY:FRAMEWORK,Foundation>" foo-needed_framework)


# feature REEXPORT_FRAMEWORK
add_library(foo-reexport_framework SHARED foo.mm)
target_link_libraries(foo-reexport_framework PRIVATE "$<LINK_LIBRARY:REEXPORT_FRAMEWORK,Foundation>")

add_executable(main-reexport_framework main.mm)
target_link_libraries(main-reexport_framework PRIVATE "$<LINK_LIBRARY:FRAMEWORK,Foundation>" foo-reexport_framework)


# feature WEAK_FRAMEWORK
add_library(foo-weak_framework SHARED foo.mm)
target_link_libraries(foo-weak_framework PRIVATE "$<LINK_LIBRARY:WEAK_FRAMEWORK,Foundation>")

add_executable(main-weak_framework main.mm)
target_link_libraries(main-weak_framework PRIVATE "$<LINK_LIBRARY:FRAMEWORK,Foundation>" foo-weak_framework)


##
## Consumption of target specified as FRAMEWORK
add_library(target-framework SHARED foo.mm)
set_target_properties(target-framework PROPERTIES FRAMEWORK TRUE)
target_link_libraries(target-framework PRIVATE "$<LINK_LIBRARY:FRAMEWORK,Foundation>")


# feature FRAMEWORK
add_executable(main-target-framework main.mm)
target_link_libraries(main-target-framework PRIVATE "$<LINK_LIBRARY:FRAMEWORK,Foundation>" "$<LINK_LIBRARY:FRAMEWORK,target-framework>")


# feature NEEDED_FRAMEWORK
add_executable(main-target-needed_framework main.mm)
target_link_libraries(main-target-needed_framework PRIVATE "$<LINK_LIBRARY:FRAMEWORK,Foundation>" "$<LINK_LIBRARY:NEEDED_FRAMEWORK,target-framework>")


# feature REEXPORT_FRAMEWORK
add_executable(main-target-reexport_framework main.mm)
target_link_libraries(main-target-reexport_framework PRIVATE "$<LINK_LIBRARY:FRAMEWORK,Foundation>" "$<LINK_LIBRARY:REEXPORT_FRAMEWORK,target-framework>")


# feature WEAK_FRAMEWORK
add_executable(main-target-weak_framework main.mm)
target_link_libraries(main-target-weak_framework PRIVATE "$<LINK_LIBRARY:FRAMEWORK,Foundation>" "$<LINK_LIBRARY:REEXPORT_FRAMEWORK,target-framework>")
