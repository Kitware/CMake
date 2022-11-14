.gitlab/ci/openmp.sh
export CMAKE_PREFIX_PATH=$PWD/.gitlab/openmp${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}
export DYLD_LIBRARY_PATH=$PWD/.gitlab/openmp/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}
