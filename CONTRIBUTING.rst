Contributing to KWSys
*********************

Overview
========

KWSys is kept in its own Git repository and shared by several projects
via copies in their source trees.  Changes to KWSys should not be made
directly in a host project, except perhaps in maintenance branches.

Please visit

  http://public.kitware.com/Wiki/KWSys/Git

to contribute changes directly to KWSys upstream.  Once changes are
reviewed, tested, and integrated there then the copies of KWSys within
dependent projects can be updated to get the changes.

Issues
======

KWSys has no independent issue tracker.  After encountering an issue
(bug) please try to submit a patch using the above instructions.
Otherwise please report the issue to the tracker for the project that
hosts the copy of KWSys in which the problem was found.

Code Style
==========

We use `clang-format`_ to define our style for C++ code in the KWSys source
tree.  See the `.clang-format`_ configuration file for our style settings.
Use ``clang-format`` version 3.8 or higher to format source files.
See also the `clang-format.bash`_ script.

.. _`clang-format`: http://clang.llvm.org/docs/ClangFormat.html
.. _`.clang-format`: .clang-format
.. _`clang-format.bash`: clang-format.bash


License
=======

We do not require any formal copyright assignment or contributor license
agreement.  Any contributions intentionally sent upstream are presumed
to be offered under terms of the OSI-approved BSD 3-clause License.
See `Copyright.txt`_ for details.

.. _`Copyright.txt`: Copyright.txt
