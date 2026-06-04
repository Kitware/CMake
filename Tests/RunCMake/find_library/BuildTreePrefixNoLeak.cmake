# Regression net for the build-tree prefix leak fixed in the Platform modules.
# When CMake runs from its build tree, CMAKE_ROOT is the source tree rather
# than an install-tree resource directory.  In that case, the historical
# dirname(dirname(CMAKE_ROOT)) install-prefix calculation must not be added to
# CMAKE_SYSTEM_PREFIX_PATH.

get_property(_cmake_running_in_build_tree GLOBAL PROPERTY
  _CMAKE_RUNNING_IN_BUILD_TREE)

if(_cmake_running_in_build_tree)
  get_filename_component(_leak_root_dir "${CMAKE_ROOT}" PATH)
  get_filename_component(_leak_prefix "${_leak_root_dir}" PATH)

    # Empty means there is no candidate.  "/" is already a standard entry, not
    # evidence of this leak.
    if(_leak_prefix AND
      NOT _leak_prefix STREQUAL "/" AND
      _leak_prefix IN_LIST CMAKE_SYSTEM_PREFIX_PATH)
    # Corner case intentionally not excluded: checking the CMake *source* tree
    # out into a system prefix such as /usr/src or /opt/<x> would make
    # _leak_prefix a standard root and trip this assertion.  Those are
    # install/system trees, not dev/CI workspaces, so the corner is treated as
    # an unsupported layout.
    message(FATAL_ERROR
      "CMAKE_SYSTEM_PREFIX_PATH leaked dirname(dirname(CMAKE_ROOT))=\"${_leak_prefix}\" "
      "(CMAKE_ROOT=\"${CMAKE_ROOT}\"); the Platform/*.cmake build-tree guard regressed.")
  endif()
endif()

unset(_cmake_running_in_build_tree)
unset(_leak_root_dir)
unset(_leak_prefix)
