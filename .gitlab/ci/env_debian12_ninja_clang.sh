if test "$CMAKE_CI_NIGHTLY" = "true"; then
  source .gitlab/ci/ticlang-env.sh
fi

export CC=/usr/bin/clang-15
export CXX=/usr/bin/clang++-15
