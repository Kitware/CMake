CMake Maintainer Guide
**********************

The following is a guide to CMake maintenance processes.
See documentation on `CMake Development`_ for more information.

.. _`CMake Development`: README.rst

.. contents:: Maintainer Processes:

Review a Merge Request
======================

The `CMake Review Process`_ requires a maintainer to issue the ``Do: merge``
command to integrate a merge request.  Please check at least the following:

* If the MR source branch (or part of it) should be backported
  to the ``release`` branch (and is already based on a commit
  contained in the ``release`` branch), add a ``Backport: release`` or
  ``Backport: release:<commit-ish>`` trailing line to the MR description.

* If the MR source branch is not named well for the change it makes
  (e.g. it is just ``master`` or the patch changed during review),
  add a ``Topic-rename: <topic>`` trailing line to the MR description
  to provide a better topic name.

* If the MR introduces a new feature or a user-facing behavior change,
  such as a policy, ensure that a ``Help/release/dev/$topic.rst`` file
  is added with a release note.

* If a commit changes a specific area, such as a module, its commit
  message should have an ``area:`` prefix on its first line.

* If a commit fixes a tracked issue, its commit message should have
  a trailing line such as ``Fixes: #00000``.

* Ensure that the MR adds sufficient documentation and test cases.

* Ensure that the MR has been tested sufficiently.  Typically it should
  be staged for nightly testing with ``Do: stage``.  Then manually
  review the `CMake CDash Page`_ to verify that no regressions were
  introduced.  (Learn to tolerate spurious failures due to idiosyncrasies
  of various nightly builders.)

* Ensure that the MR targets the ``master`` branch.  A MR intended for
  the ``release`` branch should be based on ``release`` but still target
  ``master``.  Use the above-mentioned ``Backport: release`` line to tell
  ``Do: merge`` to merge to both.  If a MR is merged without the backport
  line, a maintainer may still merge the MR topic to ``release`` manually.

Maintain Current Release
========================

The ``release`` branch is used to maintain the current release or release
candidate.  The branch is published with no version number but maintained
using a local branch named ``release-$ver``, where ``$ver`` is the version
number of the current release in the form ``$major.$minor``.  It is always
merged into ``master`` before publishing.

To merge an open MR to the ``release`` branch, edit its description to
use the ``Backport: release`` line mentioned above and then ``Do: merge``
normally.  To update the ``release`` branch manually (e.g. to merge a
``$topic`` branch that was merged without the backport line), use the
following procedure.

Before merging a ``$topic`` branch into ``release``, verify that the
``$topic`` branch has already been merged to ``master`` via the usual
``Do: merge`` process.  Then, to merge the ``$topic`` branch into
``release``, start by creating the local branch:

.. code-block:: shell

  git fetch origin
  git checkout -b release-$ver origin/release

Merge the ``$topic`` branch into the local ``release-$ver`` branch, making
sure to include a ``Merge-request: !xxxx`` footer in the commit message:

.. code-block:: shell

  git merge --no-ff $topic

Merge the ``release-$ver`` branch to ``master``:

.. code-block:: shell

  git checkout master
  git pull
  git merge --no-ff release-$ver

Review new ancestry to ensure nothing unexpected was merged to either branch:

.. code-block:: shell

  git log --graph --boundary origin/master..master
  git log --graph --boundary origin/release..release-$ver

Publish both ``master`` and ``release`` simultaneously:

.. code-block:: shell

  git push --atomic origin master release-$ver:release

.. _`CMake Review Process`: review.rst
.. _`CMake CDash Page`: https://open.cdash.org/index.php?project=CMake

Create Release Version
======================

When the ``release`` branch is ready to create a new release, follow the
steps in the above `Maintain Current Release`_ section to checkout a local
``release-$ver`` branch, where ``$ver`` is the version number of the
current release in the form ``$major.$minor``.

Edit ``Source/CMakeVersion.cmake`` to set the full version:

.. code-block:: cmake

  # CMake version number components.
  set(CMake_VERSION_MAJOR $major)
  set(CMake_VERSION_MINOR $minor)
  set(CMake_VERSION_PATCH $patch)
  #set(CMake_VERSION_RC $rc) # uncomment for release candidates

In the following we use the placeholder ``$fullver`` for the full version
number of the new release with the form ``$major.$minor.$patch[-rc$rc]``.
If the version is not a release candidate, comment out the RC version
component above and leave off the ``-rc$rc`` suffix from ``$fullver``.

Commit the release version with the **exact** message ``CMake $fullver``:

.. code-block:: shell

  git commit -m "CMake $fullver"

Tag the release using an annotated tag with the same message as the
commit and named with the **exact** form ``v$fullver``:

.. code-block:: shell

  git tag -s -m "CMake $fullver" "v$fullver"

Follow the steps in the above `Maintain Current Release`_ section to
merge the ``release-$ver`` branch into ``master`` and publish both.

Branch a New Release
====================

This section covers how to start a new ``release`` branch for a major or
minor version bump (patch releases remain on their existing branch).

In the following we use the placeholder ``$ver`` to represent the
version number of the new release with the form ``$major.$minor``,
and ``$prev`` to represent the version number of the prior release.

Review Prior Release
--------------------

Review the history around the prior release branch:

.. code-block:: shell

  git log --graph --boundary \
   ^$(git rev-list --grep="Merge topic 'doc-.*-relnotes'" -n 1 master)~1 \
   $(git rev-list --grep="Begin post-.* development" -n 1 master) \
   $(git tag --list *-rc1| tail -1)

Consolidate Release Notes
-------------------------

Starting from a clean work tree on ``master``, create a topic branch to
use for consolidating the release notes:

.. code-block:: shell

  git checkout -b doc-$ver-relnotes

Run the `consolidate-relnotes.bash`_ script:

.. code-block:: shell

  Utilities/Release/consolidate-relnotes.bash $ver $prev

.. _`consolidate-relnotes.bash`: ../../Utilities/Release/consolidate-relnotes.bash

This moves notes from the ``Help/release/dev/*.rst`` files into a versioned
``Help/release/$ver.rst`` file and updates ``Help/release/index.rst`` to
link to the new document.  Commit the changes with a message such as::

  Help: Consolidate $ver release notes

  Run the `Utilities/Release/consolidate-relnotes.bash` script to move
  notes from `Help/release/dev/*` into `Help/release/$ver.rst`.

Manually edit ``Help/release/$ver.rst`` to add section headers, organize
the notes, and revise wording.  Then commit with a message such as::

  Help: Organize and revise $ver release notes

  Add section headers similar to the $prev release notes and move each
  individual bullet into an appropriate section.  Revise a few bullets.

Update Sphinx ``versionadded`` directives in documents added since
the previous release by running the `update_versions.py`_ script:

.. code-block:: shell

  Utilities/Sphinx/update_versions.py --since v$prev.0 --overwrite

.. _`update_versions.py`: ../../Utilities/Sphinx/update_versions.py

Commit the changes with a message such as::

  Help: Update Sphinx versionadded directives for $ver release

  Run the script:

      Utilities/Sphinx/update_versions.py --since v$prev.0 --overwrite

Open a merge request with the ``doc-$ver-relnotes`` branch for review
and integration.  Further steps may proceed after this has been merged
to ``master``.

Update 'release' Branch
-----------------------

Starting from a clean work tree on ``master``, create a new ``release-$ver``
branch locally:

.. code-block:: shell

  git checkout -b release-$ver origin/master

Remove the development branch release note infrastructure:

.. code-block:: shell

  git rm Help/release/dev/0-sample-topic.rst
  sed -i '/^\.\. include:: dev.txt/ {N;d}' Help/release/index.rst

Commit with a message such as::

  Help: Drop development topic notes to prepare release

  Release versions do not have the development topic section of
  the CMake Release Notes index page.

Update ``Source/CMakeVersion.cmake`` to set the version to
``$major.$minor.0-rc0``:

.. code-block:: cmake

  # CMake version number components.
  set(CMake_VERSION_MAJOR $major)
  set(CMake_VERSION_MINOR $minor)
  set(CMake_VERSION_PATCH 0)
  set(CMake_VERSION_RC 0)

Replace uses of ``DEVEL_CMAKE_VERSION`` in the source tree with
the literal release version number string ``"$major.$minor.0"``:

.. code-block:: shell

  $EDITOR $(git grep -l DEVEL_CMAKE_VERSION)

Commit with a message such as::

  Begin $ver release versioning

Merge the ``release-$ver`` branch to ``master``:

.. code-block:: shell

  git checkout master
  git pull
  git merge --no-ff release-$ver

Begin post-release development by restoring the development branch release
note infrastructure, and the version date from ``origin/master``:

.. code-block:: shell

  git checkout origin/master -- \
    Source/CMakeVersion.cmake Help/release/dev/0-sample-topic.rst
  sed -i $'/^Releases/ i\\\n.. include:: dev.txt\\\n' Help/release/index.rst

Update ``Source/CMakeVersion.cmake`` to set the version to
``$major.$minor.$date``:

.. code-block:: cmake

  # CMake version number components.
  set(CMake_VERSION_MAJOR $major)
  set(CMake_VERSION_MINOR $minor)
  set(CMake_VERSION_PATCH $date)
  #set(CMake_VERSION_RC 0)

Commit with a message such as::

  Begin post-$ver development

Push the update to the ``master`` and ``release`` branches:

.. code-block:: shell

  git push --atomic origin master release-$ver:release

Announce 'release' Branch
-------------------------

Post a topic to the `CMake Discourse Forum Development Category`_
announcing that post-release development is open::

  I've branched `release` for $ver.  The repository is now open for
  post-$ver development.  Please rebase open merge requests on `master`
  before staging or merging.

.. _`CMake Discourse Forum Development Category`: https://discourse.cmake.org/c/development

Initial Post-Release Development
--------------------------------

Deprecate policies more than 8 release series old by updating the
policy range check in ``cmMakefile::SetPolicy``.
Commit with a message such as::

  Add deprecation warnings for policies CMP#### and below

  The OLD behaviors of all policies are deprecated, but only by
  documentation.  Add an explicit deprecation diagnostic for policies
  introduced in CMake $OLDVER and below to encourage projects to port
  away from setting policies to OLD.

Update the ``cmake_policy`` version range generated by ``install(EXPORT)``
in ``cmExportFileGenerator::GeneratePolicyHeaderCode`` to end at the
previous release.  We use one release back since we now know all the
policies added for that version.  Commit with a message such as::

  export: Increase maximum policy version in exported files to $prev

  The files generatd by `install(EXPORT)` and `export()` commands
  are known to work with policies as of CMake $prev, so enable them
  in sufficiently new CMake versions.

Update the ``cmake_minimum_required`` version range in CMake itself:

* ``CMakeLists.txt``
* ``Source/Checks/Curses/CMakeLists.txt``
* ``Utilities/Doxygen/CMakeLists.txt``
* ``Utilities/Sphinx/CMakeLists.txt``

to end in the previous release.  Commit with a message such as::

  Configure CMake itself with policies through CMake $prev
