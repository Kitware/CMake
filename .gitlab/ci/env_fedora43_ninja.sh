if test "$CMAKE_CI_NIGHTLY" = "true"; then
  source .gitlab/ci/appimagetool-env.sh
  source .gitlab/ci/ispc-env.sh
fi

# This job builds CMake as C++11.  Prevent C++17 headers from being used.
if test "$CMAKE_CONFIGURATION" = "fedora43_ninja" -a "$CI_JOB_STAGE" = "build"; then
  for i in filesystem optional string_view; do
    sed -i -e '$a#error "Use <cm/'"$i"'> instead"' "/usr/include/c++/15/$i"
  done
fi

# Test in a UTF-8 locale.
export LANG=en_US.UTF-8
