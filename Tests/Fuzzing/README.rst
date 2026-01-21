CMake Fuzzing Suite
===================

This directory contains fuzz targets for testing CMake with coverage-guided
fuzzing. These fuzzers are integrated with `OSS-Fuzz`_ for continuous fuzzing.

All fuzzers are implemented using the `libFuzzer engine`_.

.. _`OSS-Fuzz`: https://github.com/google/oss-fuzz
.. _`libFuzzer engine`: https://llvm.org/docs/LibFuzzer.html

Building Locally
----------------

To build the fuzzers locally with Clang and libFuzzer::

  mkdir build && cd build
  cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
        -DCMake_BUILD_FUZZING=ON \
        -DCMAKE_C_FLAGS="-fsanitize=fuzzer-no-link,address" \
        -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer-no-link,address" \
        ..
  make -j$(nproc)

Seed Corpora
------------

Each fuzzer has a corresponding seed corpus in ``corpus/<fuzzer_name>/``.

Dictionaries
------------

Fuzzer dictionaries are provided to improve coverage. Each fuzzer has a
corresponding ``.dict`` file with format-specific tokens.

Security Considerations
-----------------------

These fuzzers specifically target security-sensitive code paths:

- **Path traversal**: Archive extraction with ``../`` sequences
- **Symlink attacks**: File handling with symlinks
- **Memory safety**: Buffer handling in parsers
- **Integer overflows**: Size calculations in binary parsers

Bug Reports
-----------

Security bugs found by OSS-Fuzz are reported to CMake maintainers via the
OSS-Fuzz security disclosure process. Non-security bugs are filed as public
issues after a 90-day disclosure window.
