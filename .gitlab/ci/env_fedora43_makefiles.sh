if test "$CMAKE_CI_NIGHTLY" = "true"; then
  source .gitlab/ci/ispc-env.sh
fi

# Test in a Latin-1 locale.
export LANG=en_US.ISO-8859-1
