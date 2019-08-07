#!/bin/sh

sed -i -e 's/!defined(CMAKE_BUILD_WITH_CMAKE)/defined(CMAKE_BOOTSTRAP)/g' \
  -e 's/defined(CMAKE_BUILD_WITH_CMAKE)/!defined(CMAKE_BOOTSTRAP)/g' \
  -e 's/ifndef CMAKE_BUILD_WITH_CMAKE/ifdef CMAKE_BOOTSTRAP/g' \
  -e 's/ifdef CMAKE_BUILD_WITH_CMAKE/ifndef CMAKE_BOOTSTRAP/g' \
  $(git ls-files Source)
