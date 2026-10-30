#!/bin/sh

cd "$(dirname "$0")"
site=$(uname -n)
presets="$(pwd)/../Step8/CMakePresets.json"
rm -rf build

# Start a new dashboard and configure an empty build tree.
ctest -T Start -T Configure -T Submit \
  --source-dir ../Project \
  --build-dir build \
  -D CTEST_BUILD_NAME="Step9: command line" \
  -D CTEST_SITE="$site" \
  -D CTEST_PRESET=coverage \
  -D CTEST_PRESETS_FILE="$presets" \
  -D CTEST_SUBMIT_PARTS=Configure

# Report the current Git commit.
ctest -T Update -T Submit \
  --build-dir build \
  -D CTEST_UPDATE_COMMAND=git \
  -D CTEST_UPDATE_VERSION_ONLY=1 \
  -D CTEST_SUBMIT_PARTS=Update

# Build.
ctest -T Build -T Submit \
  --build-dir build \
  -D CTEST_PRESET=coverage \
  -D CTEST_PRESETS_FILE="$presets" \
  -D CTEST_USE_LAUNCHERS=1 \
  -D CTEST_SUBMIT_PARTS=Build

# Test.
ctest -T Test -T Submit \
  --build-dir build \
  -D CTEST_PRESET=coverage \
  -D CTEST_PRESETS_FILE="$presets" \
  -D CTEST_SUBMIT_PARTS=Test

# Compute coverage, and tell CDash the build is done.
ctest -T Coverage -T Submit \
  --build-dir build \
  -D CTEST_SUBMIT_PARTS="Coverage;Done"
