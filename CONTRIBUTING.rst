Contributing to CMake
*********************

Community
=========

CMake is maintained and supported by `Kitware`_ and developed in
collaboration with a productive community of contributors.
Please subscribe and post to the `CMake Developers List`_ to raise
discussion of development topics.

.. _`Kitware`: http://www.kitware.com/cmake
.. _`CMake Developers List`: https://cmake.org/mailman/listinfo/cmake-developers

Patches
=======

CMake uses `Kitware's GitLab Instance`_ to manage development and code review.
To contribute patches:

#. Fork the upstream `CMake Repository`_ into a personal account.
#. Base all new work on the upstream ``master`` branch.
#. Create commits making incremental, distinct, logically complete changes.
#. Push a topic branch to a personal repository fork on GitLab.
#. Create a GitLab Merge Request targeting the upstream ``master`` branch.

.. _`Kitware's GitLab Instance`: https://gitlab.kitware.com
.. _`CMake Repository`: https://gitlab.kitware.com/cmake/cmake

Code Style
==========

We use `clang-format`_ to define our style for C++ code in the CMake source
tree.  See the `.clang-format`_ configuration file for our style settings.
Use ``clang-format`` version 3.8 or higher to format source files.
See also the `Utilities/Scripts/clang-format.bash`_ script.

.. _`clang-format`: http://clang.llvm.org/docs/ClangFormat.html
.. _`.clang-format`: .clang-format
.. _`Utilities/Scripts/clang-format.bash`: Utilities/Scripts/clang-format.bash

License
=======

We do not require any formal copyright assignment or contributor license
agreement.  Any contributions intentionally sent upstream are presumed
to be offered under terms of the OSI-approved BSD 3-clause License.
See `Copyright.txt`_ for details.

.. _`Copyright.txt`: Copyright.txt
