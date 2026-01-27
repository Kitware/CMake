set_property(GLOBAL PROPERTY JOB_POOLS source_file_compile_pool=2 target_compile_pool=4 target_link_pool=1)

enable_language(C)

set_property(SOURCE hello.c PROPERTY JOB_POOL_COMPILE "source_file_compile_pool")

add_executable(hello hello.c)
set_property(TARGET hello PROPERTY JOB_POOL_COMPILE "target_compile_pool")
set_property(TARGET hello PROPERTY JOB_POOL_LINK "target_link_pool")

include(CheckNoPrefixSubDir.cmake)
