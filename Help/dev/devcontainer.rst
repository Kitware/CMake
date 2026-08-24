CMake Dev Container Guide
*************************

The following is a guide to the development container provided for building,
testing, and formatting CMake itself.  See documentation on `CMake
Development`_ for more information.

.. _`CMake Development`: README.rst

Overview
========

The `.devcontainer`_ directory at the top of the CMake source tree describes
a Linux development environment following the `Dev Container Specification`_.
Using it is entirely optional, but it offers a quick way to get a complete
environment with all the tools needed to build CMake, run its test suite,
build its documentation, and satisfy its style rules.

The container is built on Ubuntu, which offers the broadest ecosystem of
packages and tooling for development.  Its package lists mirror those of the
Debian image our CI infrastructure uses, described under
`.gitlab/ci/docker`_, so the dependencies available closely match the ones
against which merge requests are tested.  A few pieces of the CI environment
are left out because a development container rarely needs them, and each is
noted in the list that would otherwise carry it:

`.devcontainer/deps_packages.lst`_
  The packages needed to build CMake, run its test suite, and build its
  documentation.

`.devcontainer/dev_packages.lst`_
  The packages that make the container a comfortable place to work in, which
  building and testing CMake does not itself need.

.. _`.devcontainer`: ../../.devcontainer
.. _`Dev Container Specification`: https://containers.dev
.. _`.gitlab/ci/docker`: ../../.gitlab/ci/docker
.. _`.devcontainer/deps_packages.lst`: ../../.devcontainer/deps_packages.lst
.. _`.devcontainer/dev_packages.lst`: ../../.devcontainer/dev_packages.lst

Prerequisites
=============

* A container engine such as `Docker`_ or `Podman`_.  Building the image uses
  bind and cache mounts, as the image builds under `.gitlab/ci/docker`_ do, so
  it needs `BuildKit`_, enabled by default since Docker 23.0, or Podman 4.0 or
  newer.

* A tool that understands the specification, such as the `Dev Containers`_
  extension for Visual Studio Code, the `Dev Container CLI`_, or another
  `supporting tool`_.

.. _`Docker`: https://docs.docker.com/get-started/get-docker/
.. _`Podman`: https://podman.io
.. _`BuildKit`: https://docs.docker.com/build/buildkit/
.. _`Dev Containers`: https://code.visualstudio.com/docs/devcontainers/containers
.. _`Dev Container CLI`: https://github.com/devcontainers/cli
.. _`supporting tool`: https://containers.dev/supporting

Usage
=====

In Visual Studio Code, open the CMake source tree and run the
``Dev Containers: Reopen in Container`` command.  With the
`Dev Container CLI`_, start the container from the top of the source tree:

.. code-block:: console

  $ devcontainer up --workspace-folder .
  $ devcontainer exec --workspace-folder . bash

The source tree is mounted into the container, so changes made inside it are
made to the same working tree.  Commits may be created either inside or
outside the container.  `Utilities/SetupForDevelopment.sh`_ may likewise be
run in either place to configure your Git identity and install the project's
commit hooks, and takes effect in both.  It is interactive, so the container
does not run it automatically, but `.devcontainer/setup-status.sh`_ reports
whether it still needs to be run each time a tool attaches to the container.

.. _`Utilities/SetupForDevelopment.sh`: ../../Utilities/SetupForDevelopment.sh
.. _`.devcontainer/setup-status.sh`: ../../.devcontainer/setup-status.sh

Build CMake in the container as one would on any other Linux host, as
described in `Building CMake`_:

.. code-block:: console

  $ cmake -G Ninja -B build -S .
  $ cmake --build build
  $ ctest --test-dir build

.. _`Building CMake`: ../../README.rst#building-cmake

Provided Tools
==============

In addition to the compiler and the external dependencies CMake can build
against, the container provides:

* ``cmake`` and ``ninja``, to build CMake with.  ``cmake`` comes from the
  `Kitware APT repository`_, which the container configures, so it is the
  latest CMake release rather than the older one Ubuntu carries, and
  ``apt-get`` offers each new release as it is published:

  .. code-block:: console

    $ sudo apt-get update
    $ sudo apt-get install --only-upgrade cmake

  The repository also carries release candidates, in a suite named after the
  Ubuntu release with ``-rc`` appended.  Add it to the ``Suites`` field of
  ``/etc/apt/sources.list.d/kitware.sources`` to install those as well.

* ``ccache``, to speed up repeated builds, e.g.:

  .. code-block:: console

    $ cmake -G Ninja -B build -S . -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

  Its cache is stored in a named volume so that it survives rebuilds of the
  container.

* ``clang-format`` version 18, exactly as required by our `C++ Code Style`_,
  available as both ``clang-format`` and ``clang-format-18``:

  .. code-block:: console

    $ Utilities/Scripts/clang-format.bash --modified

* ``pre-commit``, to run the checks configured in
  `.pre-commit-config.yaml`_:

  .. code-block:: console

    $ pre-commit install
    $ pre-commit run --all-files

* ``sphinx-build``, to build the documentation as described in the
  `CMake Documentation Guide`_.

* ``gdb``, to debug CMake as described in the `CMake Debugging Guide`_.

* ``glab``, the `GitLab CLI`_, to work with merge requests, issues, and
  pipelines on our GitLab instance, and `glab-axi`_, a wrapper around it
  whose output follows the `AXI`_ conventions:

  .. code-block:: console

    $ glab mr list
    $ glab-axi mr view 1234

  See `GitLab Authentication`_ below for the one-time setup they need.

.. _`Kitware APT repository`: https://apt.kitware.com
.. _`C++ Code Style`: source.rst#c-code-style
.. _`.pre-commit-config.yaml`: ../../.pre-commit-config.yaml
.. _`CMake Documentation Guide`: documentation.rst
.. _`CMake Debugging Guide`: debug.rst
.. _`GitLab CLI`: https://docs.gitlab.com/editor_extensions/gitlab_cli/
.. _`glab-axi`: https://github.com/karotkriss/glab-axi
.. _`AXI`: https://axi.md

The base image ships without documentation, but the container keeps the man
pages and other documentation of every package installed on top of it.  Run
``sudo unminimize`` to restore the documentation of the packages the base
image itself provides.

GitLab Authentication
=====================

The container sets ``GITLAB_HOST`` to ``gitlab.kitware.com`` so that ``glab``
and ``glab-axi`` address our GitLab instance by default.  Both still need a
credential for it.  `.devcontainer/setup-status.sh`_ reports whether a
working credential has been configured and provides instructions to do so if
not.  It is run automatically when attaching to the container.

A ``GITLAB_TOKEN`` or ``GITLAB_CLIENT_ID`` set on the host is passed through
to the container, so a credential configured outside it is used as-is.

Local Customization
===================

The container is meant to be an unconstrained space that each developer may
adapt.  Two optional scripts, if present, run while the container image is
built:

``.devcontainer/hooks/root.sh``
  Runs as ``root``, e.g. to install additional packages.

``.devcontainer/hooks/user.sh``
  Runs as the unprivileged container user, e.g. to populate a shell
  configuration file.

Each runs in the home directory of the user it runs as, with ``HOME`` naming
that directory.

The whole ``hooks`` directory is ignored by Git, apart from its
``.gitignore``, so customizations never appear in a commit, may bring along
whatever other files they need, and are preserved across updates to the
tracked container definition.  For example, to add a package and a shell
alias:

.. code-block:: console

  $ cat > .devcontainer/hooks/root.sh <<'EOF'
  apt-get update && apt-get install -y tmux
  EOF
  $ cat > .devcontainer/hooks/user.sh <<'EOF'
  echo "alias b='cmake --build build'" >> ~/.bashrc
  EOF

Rebuild the container to apply them, e.g. with the
``Dev Containers: Rebuild Container`` command in Visual Studio Code.

Larger or longer-lived changes may of course be made by editing
`.devcontainer/Dockerfile`_ or `.devcontainer/devcontainer.json`_ directly,
but take care not to commit them accidentally.

.. _`.devcontainer/Dockerfile`: ../../.devcontainer/Dockerfile
.. _`.devcontainer/devcontainer.json`: ../../.devcontainer/devcontainer.json
