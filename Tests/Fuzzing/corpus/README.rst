Fuzzing Seed Corpora
====================

This directory contains seed corpus files for the CMake fuzz targets.
Each subdirectory corresponds to a specific fuzzer.

Regenerating Binary Corpus Files
--------------------------------

Some corpus files are binary and include generation scripts for reproducibility:

- ``elf/generate.py`` - Generates minimal ELF headers for cmELFFuzzer

To regenerate::

    cd elf && python3 generate.py

Adding New Corpus Files
-----------------------

When adding new corpus files:

1. Keep files minimal - smaller seeds lead to faster fuzzing
2. For binary formats, include a generation script
3. Cover different code paths and edge cases
4. Avoid duplicating coverage between files
