#!/usr/bin/env bash

set -e

if test "$CI_COMMIT_REF_NAME" != "master"; then
  echo "The version update may run only on the 'master' branch."
  exit 1
fi

# A project-specific access token must be provided by
# the pipeline schedule under which this job runs.
if test -z "$CMAKE_CI_GIT_ACCESS_TOKEN"; then
  echo "No CMAKE_CI_GIT_ACCESS_TOKEN is available."
  exit 1
fi

git config user.name "${CMAKE_CI_AUTHOR_NAME-Kitware Robot}"
git config user.email "${CMAKE_CI_AUTHOR_EMAIL-kwrobot@kitware.com}"
git remote add upstream "https://oauth2:$CMAKE_CI_GIT_ACCESS_TOKEN@gitlab.kitware.com/$CI_PROJECT_PATH.git"

# Repeat a few times in case we lose a race.
n=6
for try in $(seq $n); do
  git fetch upstream "$CI_COMMIT_REF_NAME"
  git reset -q --hard FETCH_HEAD
  Source/CMakeVersion.bash
  git update-index -q --ignore-missing --refresh
  modified=$(git diff-index --name-only HEAD -- "Source/CMakeVersion.cmake")
  if test -n "$modified"; then
    echo "version changed"
    git add -u
    git commit -m "CMake Nightly Date Stamp"
    if git push --push-option=ci.skip upstream "HEAD:$CI_COMMIT_REF_NAME"; then
      exit 0
    else
      echo "Try #$try failed to fast-forward."
    fi
  else
    echo "version unchanged"
    exit 0
  fi
  sleep 30
done

# Give up after failing too many times.
exit 1
