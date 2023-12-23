enable_language(C)

add_library(hello STATIC hello.c)

target_link_options(hello PRIVATE "-FLAGS=[
  FLAG1,
  FLAG2]")
