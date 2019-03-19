enable_language(CXX)

set(DEPLOY_DIR
   "temp\\foodir"
)

add_library(foo SHARED foo.cpp)

set_target_properties(foo
 PROPERTIES
  DEPLOYMENT_ADDITIONAL_FILES "foo.dll|\\foo\\src\\dir\\on\\host|$(RemoteDirectory)|0;bar.dll|\\bar\\src\\dir|$(RemoteDirectory)bardir|0"
  DEPLOYMENT_REMOTE_DIRECTORY ${DEPLOY_DIR}
  VS_NO_SOLUTION_DEPLOY $<CONFIG:Release>
)
