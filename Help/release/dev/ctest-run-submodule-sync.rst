ctest-run-submodule-sync
------------------------

* The :command:`ctest_update` command now looks at the
  :variable:`CTEST_GIT_INIT_SUBMODULES` variable to determine whether
  submodules should be updated or not before updating.
* The :command:`ctest_update` command will now synchronize submodules on an
  update. Updates which add submodules or change a submodule's URL will now be
  pulled properly.
