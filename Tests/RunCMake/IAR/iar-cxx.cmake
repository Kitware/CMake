enable_language(CXX)

add_executable(exec-cxx module.cxx)
target_link_options(exec-cxx PRIVATE ${LINKER_OPTS})
