enable_language(CXX)

add_executable(exec-cxx)
target_sources(exec-cxx PRIVATE module.cxx)
target_link_options(exec-cxx PRIVATE ${LINKER_OPTS})
