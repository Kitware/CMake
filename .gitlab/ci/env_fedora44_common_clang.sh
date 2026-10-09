export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
export FC=/usr/bin/flang

if test "$CMAKE_CI_NIGHTLY" = "true"; then
  source .gitlab/ci/acpp-env.sh
fi
