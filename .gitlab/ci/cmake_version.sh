#!/usr/bin/env bash
cmake -P "${BASH_SOURCE%/*}/cmake_version.cmake" | cut -d ' ' -f 2
