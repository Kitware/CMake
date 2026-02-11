if test "$CMAKE_CI_NIGHTLY" = "true"; then
  source .gitlab/ci/appimagetool-env.sh
  source .gitlab/ci/ispc-env.sh
fi

# Test in a UTF-8 locale.
export LANG=en_US.UTF-8
