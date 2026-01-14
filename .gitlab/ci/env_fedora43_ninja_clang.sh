if test "$CMAKE_CI_NIGHTLY" = "true"; then
  source .gitlab/ci/appimagetool-env.sh
  source .gitlab/ci/ispc-env.sh
fi

. .gitlab/ci/env_fedora43_common_clang.sh
