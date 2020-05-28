
add_library (foo1 INTERFACE)
add_library (foo2 INTERFACE)

add_library (imp1 SHARED IMPORTED GLOBAL)
add_library (imp2 SHARED IMPORTED GLOBAL)


add_library (alias1 ALIAS foo1)
# same alias to different library
add_library (alias1 ALIAS foo2)
# same alias to different imported library
add_library (alias1 ALIAS imp1)


add_library (alias2 ALIAS imp1)
# same alias to different imported library
add_library (alias2 ALIAS imp2)
# same alias to different library
add_library (alias2 ALIAS foo1)
