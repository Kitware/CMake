block()
  cmake_policy(SET CMP0147 NEW) # Build custom commands in parallel by default

  add_custom_command(OUTPUT "foo.out.txt" COMMAND echo Foo > foo.out.txt MAIN_DEPENDENCY "foo.txt")
  add_custom_command(OUTPUT "bar.out.txt" COMMAND echo Bar > bar.out.txt MAIN_DEPENDENCY "bar.txt")
  set_property(SOURCE "bar.txt" PROPERTY VS_CUSTOM_COMMAND_DISABLE_PARALLEL_BUILD TRUE)

  add_custom_target(foo1 SOURCES foo.txt)
  add_custom_target(bar1 SOURCES bar.txt)
endblock()

block()
  cmake_policy(SET CMP0147 OLD) # Don't build custom commands in parallel by default

  add_custom_command(OUTPUT "foo.out.cpp" COMMAND echo Foo > foo.out.txt MAIN_DEPENDENCY "foo.cpp")
  add_custom_command(OUTPUT "bar.out.cpp" COMMAND echo Bar > bar.out.txt MAIN_DEPENDENCY "bar.cpp")
  set_property(SOURCE "bar.cpp" PROPERTY VS_CUSTOM_COMMAND_DISABLE_PARALLEL_BUILD TRUE)

  add_custom_target(foo2 SOURCES foo.cpp)
  add_custom_target(bar2 SOURCES bar.cpp)
endblock()
