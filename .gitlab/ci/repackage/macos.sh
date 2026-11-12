#!/usr/bin/env bash

set -e

sdkPath="$(xcrun --show-sdk-path)"
sdkVers="$(xcrun --show-sdk-version)"

tar cjf "MacOSX${sdkVers}.sdk.tar.bz2" -C "${sdkPath%/*}" --no-fflags "MacOSX.sdk"
