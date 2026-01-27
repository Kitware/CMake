enable_language(CXX)

set(DEPLOY_DIR
   "temp\\foodir"
)

add_library(foo SHARED foo.cpp)

set_target_properties(foo
 PROPERTIES
  VS_SOLUTION_DEPLOY $<NOT:$<CONFIG:Release>>
)
