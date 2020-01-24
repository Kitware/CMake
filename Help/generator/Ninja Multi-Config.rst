Ninja Multi-Config
------------------

Generates multiple ``build-<Config>.ninja`` files.

This generator is very much like the :generator:`Ninja` generator, but with
some key differences. Only these differences will be discussed in this
document.

Unlike the :generator:`Ninja` generator, ``Ninja Multi-Config`` generates
multiple configurations at once with :variable:`CMAKE_CONFIGURATION_TYPES`
instead of only one configuration with :variable:`CMAKE_BUILD_TYPE`. One
``build-<Config>.ninja`` file will be generated for each of these
configurations (with ``<Config>`` being the configuration name.) No
``build.ninja`` file is generated, unless
:variable:`CMAKE_NINJA_MULTI_DEFAULT_BUILD_TYPE` is specified. You must specify
the desired ``build-<Config>.ninja`` file with ``ninja -f``. Running
``cmake --build . --config <Config> --target <target>`` will run Ninja with
``build-<Config>.ninja`` as the ``-f`` file and ``<target>`` as the build
target.

If :variable:`CMAKE_NINJA_MULTI_CROSS_CONFIG_ENABLE` is turned on, executables
and libraries of any configuration can be built regardless of which
``build-<Config>.ninja`` file is used, simply by specifying
``<target>:<OtherConfig>`` as the Ninja target. You can also specify
``<target>:all`` to build a target in all configurations. Each
``build-<Config>.ninja`` file will additionally have ``<target>`` targets which
are aliases for ``<target>:<Config>``. However, custom commands and custom
targets will always use the configuration specified in
``build-<Config>.ninja``. This is due to the fact that it is impossible in
Ninja for the same file to be output with different commands in the same build
graph.

You can additionally use :variable:`CMAKE_NINJA_MULTI_CROSS_CONFIGS` to limit
the configurations that get cross-generated. If this variable is set, each
``build-<Config>.ninja`` file will only contain rules for the configurations
listed in the variable, plus their own configuration. This also affects which
configurations are built by the ``<target>:all`` target.

If :variable:`CMAKE_NINJA_MULTI_CROSS_CONFIG_ENABLE` is not enabled, you can
still build any target in ``build-<Config>.ninja`` by specifying
``<target>:<Config>`` or ``<target>``, but not ``<target>:<OtherConfig>`` or
``<target>:all``.

Consider the following example:

.. code-block:: cmake

  cmake_minimum_required(VERSION 3.16)
  project(MultiConfigNinja C)

  add_executable(generator generator.c)
  add_custom_command(OUTPUT generated.c COMMAND generator generated.c)
  add_library(generated ${CMAKE_BINARY_DIR}/generated.c)

Now assume you configure the project with ``Ninja Multi-Config`` and run one of
the following commands:

.. code-block:: shell

  ninja -f build-Debug.ninja generated
  # OR
  cmake --build . --config Debug --target generated

This would build the ``Debug`` configuration of ``generator``, which would be
used to generate ``generated.c``, which would be used to build the ``Debug``
configuration of ``generated``.

But if :variable:`CMAKE_NINJA_MULTI_CROSS_CONFIG_ENABLE` is enabled, and you
run the following instead:

.. code-block:: shell

  ninja -f build-Release.ninja generated:Debug
  # OR
  cmake --build . --config Release --target generated:Debug

This would build the ``Release`` configuration of ``generator``, which would be
used to generate ``generated.c``, which would be used to build the ``Debug``
configuration of ``generated``. This is useful for running a release-optimized
version of a generator utility while still building the debug version of the
targets built with the generated code.

As a convenience, ``Ninja Multi-Config`` offers a
:variable:`CMAKE_NINJA_MULTI_DEFAULT_BUILD_TYPE` setting. If this variable is
specified, a ``build.ninja`` file will be generated which points to the
specified ``build-<Config>.ninja`` file. In addition, if
:variable:`CMAKE_NINJA_MULTI_DEFAULT_BUILD_TYPE` is used in conjunction with
:variable:`CMAKE_NINJA_MULTI_CROSS_CONFIG_ENABLE`, you can also specify
:variable:`CMAKE_NINJA_MULTI_DEFAULT_BUILD_ALIAS`, which changes the config
of the ``<target>`` targets in ``build.ninja``. For example, if you set
:variable:`CMAKE_NINJA_MULTI_DEFAULT_BUILD_TYPE` to ``Release``, but set
:variable:`CMAKE_NINJA_MULTI_DEFAULT_BUILD_ALIAS` to ``Debug`` or ``all``,
all ``<target>`` aliases in ``build.ninja`` will resolve to ``<target>:Debug``
or ``<target>:all``, but custom commands will still use the ``Release``
configuration.
