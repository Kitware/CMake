target_link_libraries(top INTERFACE debug "$<1:foo;foo>" optimized "$<1:foo;foo>")
