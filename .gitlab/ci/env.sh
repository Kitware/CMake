quietly() {
  readonly log="/tmp/quietly-$RANDOM.log"
  if ! "$@" >"$log" 2>&1; then
    ret=$?
    cat "$log"
    rm -f "$log"
    exit $ret
  fi
  rm -f "$log"
}

if test -n "$CMAKE_CI_IN_SYMLINK_TREE"; then
  mkdir -p "$CI_PROJECT_DIR/real_work/work/build"
  ln -s real_work/work "$CI_PROJECT_DIR/work"
  git worktree prune
  git worktree add "$CI_PROJECT_DIR/work/cmake" HEAD

  # Assert that the hash matches.
  test "$(git -C "$CI_PROJECT_DIR/work/cmake" rev-parse HEAD)" = "$(git -C "$CI_PROJECT_DIR" rev-parse HEAD)"
fi

if test -r ".gitlab/ci/env_${CMAKE_CONFIGURATION}.sh"; then
  source ".gitlab/ci/env_${CMAKE_CONFIGURATION}.sh"
fi
