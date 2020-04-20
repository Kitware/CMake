add_library(empty)
target_sources(empty PRIVATE $<HOST_LINK:empty.c>)
