set(PROCESSORS 1)
set(RUN_SHELL "/usr/local/bin/zsh -l -c /bin/sh")
set(CVS_COMMAND "/usr/local/bin/cvs")
set(HOST destiny)
set(MAKE_PROGRAM "/usr/local/bin/gmake")
set(INITIAL_CACHE "CMAKE_BUILD_TYPE:STRING=Release
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
CMAKE_EXE_LINKER_FLAGS:STRING=-Wl,-a,archive_shared
CMAKE_C_FLAGS:STRING=+DAportable
CMAKE_CXX_FLAGS:STRING=-Wl,+vnocompatwarnings +W740,749 +DAportable -D__HPACC_STRICTER_ANSI__")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
