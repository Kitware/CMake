source .gitlab/ci/fastbuild-env.sh
if test "$CMAKE_CI_NIGHTLY" = "true"; then
  source .gitlab/ci/acpp-env.sh
fi
