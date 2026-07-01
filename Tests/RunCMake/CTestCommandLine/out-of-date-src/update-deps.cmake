set(src_dir "${CMAKE_CURRENT_LIST_DIR}")

file(APPEND "${src_dir}/main.c" "\n/* update main */\n")
file(APPEND "${src_dir}/lib.c" "\n/* update lib */\n")
