PROJECT_IS_TOP_LEVEL
--------------------

.. versionadded:: 3.21

A boolean variable indicating whether :command:`project` was called in a top
level ``CMakeLists.txt`` file.

Some modules should only be included as part of the top level
``CMakeLists.txt`` file to not cause unintended side effects in the build
tree, and this variable can be used to conditionally execute such code. For
example, consider the :module:`CTest` module, which creates targets and
options:

.. code-block:: cmake

  project(MyProject)
  ...
  if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
  endif()
