cmake \
  -S Utilities/ClangTidyModule \
  -B Utilities/ClangTidyModule/build \
  -DRUN_TESTS=ON
cmake --build Utilities/ClangTidyModule/build
ctest --test-dir Utilities/ClangTidyModule/build
