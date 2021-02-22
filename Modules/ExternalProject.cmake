# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

include_guard(GLOBAL)

#[=======================================================================[.rst:
ExternalProject
---------------

.. only:: html

   .. contents::

Commands
^^^^^^^^

External Project Definition
"""""""""""""""""""""""""""

.. command:: ExternalProject_Add

  The ``ExternalProject_Add()`` function creates a custom target to drive
  download, update/patch, configure, build, install and test steps of an
  external project:

  .. code-block:: cmake

    ExternalProject_Add(<name> [<option>...])

  The individual steps within the process can be driven independently if
  required (e.g. for CDash submission) and extra custom steps can be defined,
  along with the ability to control the step dependencies. The directory
  structure used for the management of the external project can also be
  customized. The function supports a large number of options which can be used
  to tailor the external project behavior.

  **Directory Options:**
    Most of the time, the default directory layout is sufficient. It is largely
    an implementation detail that the main project usually doesn't need to
    change. In some circumstances, however, control over the directory layout
    can be useful or necessary. The directory options are potentially more
    useful from the point of view that the main build can use the
    :command:`ExternalProject_Get_Property` command to retrieve their values,
    thereby allowing the main project to refer to build artifacts of the
    external project.

    ``PREFIX <dir>``
      Root directory for the external project. Unless otherwise noted below,
      all other directories associated with the external project will be
      created under here.

    ``TMP_DIR <dir>``
      Directory in which to store temporary files.

    ``STAMP_DIR <dir>``
      Directory in which to store the timestamps of each step. Log files from
      individual steps are also created in here unless overridden by LOG_DIR
      (see *Logging Options* below).

    ``LOG_DIR <dir>``
      .. versionadded:: 3.14

      Directory in which to store the logs of each step.

    ``DOWNLOAD_DIR <dir>``
      Directory in which to store downloaded files before unpacking them. This
      directory is only used by the URL download method, all other download
      methods use ``SOURCE_DIR`` directly instead.

    ``SOURCE_DIR <dir>``
      Source directory into which downloaded contents will be unpacked, or for
      non-URL download methods, the directory in which the repository should be
      checked out, cloned, etc. If no download method is specified, this must
      point to an existing directory where the external project has already
      been unpacked or cloned/checked out.

      .. note::
         If a download method is specified, any existing contents of the source
         directory may be deleted. Only the URL download method checks whether
         this directory is either missing or empty before initiating the
         download, stopping with an error if it is not empty. All other
         download methods silently discard any previous contents of the source
         directory.

    ``BINARY_DIR <dir>``
      Specify the build directory location. This option is ignored if
      ``BUILD_IN_SOURCE`` is enabled.

    ``INSTALL_DIR <dir>``
      Installation prefix to be placed in the ``<INSTALL_DIR>`` placeholder.
      This does not actually configure the external project to install to
      the given prefix. That must be done by passing appropriate arguments
      to the external project configuration step, e.g. using ``<INSTALL_DIR>``.

    If any of the above ``..._DIR`` options are not specified, their defaults
    are computed as follows. If the ``PREFIX`` option is given or the
    ``EP_PREFIX`` directory property is set, then an external project is built
    and installed under the specified prefix::

      TMP_DIR      = <prefix>/tmp
      STAMP_DIR    = <prefix>/src/<name>-stamp
      DOWNLOAD_DIR = <prefix>/src
      SOURCE_DIR   = <prefix>/src/<name>
      BINARY_DIR   = <prefix>/src/<name>-build
      INSTALL_DIR  = <prefix>
      LOG_DIR      = <STAMP_DIR>

    Otherwise, if the ``EP_BASE`` directory property is set then components
    of an external project are stored under the specified base::

      TMP_DIR      = <base>/tmp/<name>
      STAMP_DIR    = <base>/Stamp/<name>
      DOWNLOAD_DIR = <base>/Download/<name>
      SOURCE_DIR   = <base>/Source/<name>
      BINARY_DIR   = <base>/Build/<name>
      INSTALL_DIR  = <base>/Install/<name>
      LOG_DIR      = <STAMP_DIR>

    If no ``PREFIX``, ``EP_PREFIX``, or ``EP_BASE`` is specified, then the
    default is to set ``PREFIX`` to ``<name>-prefix``. Relative paths are
    interpreted with respect to :variable:`CMAKE_CURRENT_BINARY_DIR` at the
    point where ``ExternalProject_Add()`` is called.

  **Download Step Options:**
    A download method can be omitted if the ``SOURCE_DIR`` option is used to
    point to an existing non-empty directory. Otherwise, one of the download
    methods below must be specified (multiple download methods should not be
    given) or a custom ``DOWNLOAD_COMMAND`` provided.

    ``DOWNLOAD_COMMAND <cmd>...``
      Overrides the command used for the download step
      (:manual:`generator expressions <cmake-generator-expressions(7)>` are
      supported). If this option is specified, all other download options will
      be ignored. Providing an empty string for ``<cmd>`` effectively disables
      the download step.

    *URL Download*
      ``URL <url1> [<url2>...]``
        List of paths and/or URL(s) of the external project's source. When more
        than one URL is given, they are tried in turn until one succeeds. A URL
        may be an ordinary path in the local file system (in which case it
        must be the only URL provided) or any downloadable URL supported by the
        :command:`file(DOWNLOAD)` command. A local filesystem path may refer to
        either an existing directory or to an archive file, whereas a URL is
        expected to point to a file which can be treated as an archive. When an
        archive is used, it will be unpacked automatically unless the
        ``DOWNLOAD_NO_EXTRACT`` option is set to prevent it. The archive type
        is determined by inspecting the actual content rather than using logic
        based on the file extension.

        .. versionchanged:: 3.7
          Multiple URLs are allowed.

      ``URL_HASH <algo>=<hashValue>``
        Hash of the archive file to be downloaded. The argument should be of
        the form ``<algo>=<hashValue>`` where ``algo`` can be any of the hashing
        algorithms supported by the :command:`file()` command. Specifying this
        option is strongly recommended for URL downloads, as it ensures the
        integrity of the downloaded content. It is also used as a check for a
        previously downloaded file, allowing connection to the remote location
        to be avoided altogether if the local directory already has a file from
        an earlier download that matches the specified hash.

      ``URL_MD5 <md5>``
        Equivalent to ``URL_HASH MD5=<md5>``.

      ``DOWNLOAD_NAME <fname>``
        File name to use for the downloaded file. If not given, the end of the
        URL is used to determine the file name. This option is rarely needed,
        the default name is generally suitable and is not normally used outside
        of code internal to the ``ExternalProject`` module.

      ``DOWNLOAD_NO_EXTRACT <bool>``
        .. versionadded:: 3.6

        Allows the extraction part of the download step to be disabled by
        passing a boolean true value for this option. If this option is not
        given, the downloaded contents will be unpacked automatically if
        required. If extraction has been disabled, the full path to the
        downloaded file is available as ``<DOWNLOADED_FILE>`` in subsequent
        steps or as the property ``DOWNLOADED_FILE`` with the
        :command:`ExternalProject_Get_Property` command.

      ``DOWNLOAD_NO_PROGRESS <bool>``
        Can be used to disable logging the download progress. If this option is
        not given, download progress messages will be logged.

      ``TIMEOUT <seconds>``
        Maximum time allowed for file download operations.

      ``INACTIVITY_TIMEOUT <seconds>``
        .. versionadded:: 3.19

        Terminate the operation after a period of inactivity.

      ``HTTP_USERNAME <username>``
        .. versionadded:: 3.7

        Username for the download operation if authentication is required.

      ``HTTP_PASSWORD <password>``
        .. versionadded:: 3.7

        Password for the download operation if authentication is required.

      ``HTTP_HEADER <header1> [<header2>...]``
        .. versionadded:: 3.7

        Provides an arbitrary list of HTTP headers for the download operation.
        This can be useful for accessing content in systems like AWS, etc.

      ``TLS_VERIFY <bool>``
        Specifies whether certificate verification should be performed for
        https URLs. If this option is not provided, the default behavior is
        determined by the ``CMAKE_TLS_VERIFY`` variable (see
        :command:`file(DOWNLOAD)`). If that is also not set, certificate
        verification will not be performed. In situations where ``URL_HASH``
        cannot be provided, this option can be an alternative verification
        measure.

        .. versionchanged:: 3.6
          This option also applies to ``git clone`` invocations.

      ``TLS_CAINFO <file>``
        Specify a custom certificate authority file to use if ``TLS_VERIFY``
        is enabled. If this option is not specified, the value of the
        ``CMAKE_TLS_CAINFO`` variable will be used instead (see
        :command:`file(DOWNLOAD)`)

      ``NETRC <level>``
        .. versionadded:: 3.11

        Specify whether the ``.netrc`` file is to be used for operation.
        If this option is not specified, the value of the ``CMAKE_NETRC``
        variable will be used instead (see :command:`file(DOWNLOAD)`)
        Valid levels are:

        ``IGNORED``
          The ``.netrc`` file is ignored.
          This is the default.
        ``OPTIONAL``
          The ``.netrc`` file is optional, and information in the URL
          is preferred.  The file will be scanned to find which ever
          information is not specified in the URL.
        ``REQUIRED``
          The ``.netrc`` file is required, and information in the URL
          is ignored.

      ``NETRC_FILE <file>``
        .. versionadded:: 3.11

        Specify an alternative ``.netrc`` file to the one in your home directory
        if the ``NETRC`` level is ``OPTIONAL`` or ``REQUIRED``. If this option
        is not specified, the value of the ``CMAKE_NETRC_FILE`` variable will
        be used instead (see :command:`file(DOWNLOAD)`)

      .. versionadded:: 3.1
        Added support for `tbz2`, `.tar.xz`, `.txz`, and `.7z` extensions.

    *Git*
      NOTE: A git version of 1.6.5 or later is required if this download method
      is used.

      ``GIT_REPOSITORY <url>``
        URL of the git repository. Any URL understood by the ``git`` command
        may be used.

      ``GIT_TAG <tag>``
        Git branch name, tag or commit hash. Note that branch names and tags
        should generally be specified as remote names (i.e. ``origin/myBranch``
        rather than simply ``myBranch``). This ensures that if the remote end
        has its tag moved or branch rebased or history rewritten, the local
        clone will still be updated correctly. In general, however, specifying
        a commit hash should be preferred for a number of reasons:

        - If the local clone already has the commit corresponding to the hash,
          no ``git fetch`` needs to be performed to check for changes each time
          CMake is re-run. This can result in a significant speed up if many
          external projects are being used.
        - Using a specific git hash ensures that the main project's own history
          is fully traceable to a specific point in the external project's
          evolution. If a branch or tag name is used instead, then checking out
          a specific commit of the main project doesn't necessarily pin the
          whole build to a specific point in the life of the external project.
          The lack of such deterministic behavior makes the main project lose
          traceability and repeatability.

        If ``GIT_SHALLOW`` is enabled then ``GIT_TAG`` works only with
        branch names and tags.  A commit hash is not allowed.

      ``GIT_REMOTE_NAME <name>``
        The optional name of the remote. If this option is not specified, it
        defaults to ``origin``.

      ``GIT_SUBMODULES <module>...``
        Specific git submodules that should also be updated. If this option is
        not provided, all git submodules will be updated.

        .. versionchanged:: 3.16
          When :policy:`CMP0097` is set to ``NEW``, if this value is set
          to an empty string then no submodules are initialized or updated.

      ``GIT_SUBMODULES_RECURSE <bool>``
        .. versionadded:: 3.17

        Specify whether git submodules (if any) should update recursively by
        passing the ``--recursive`` flag to ``git submodule update``.
        If not specified, the default is on.

      ``GIT_SHALLOW <bool>``
        .. versionadded:: 3.6

        When this option is enabled, the ``git clone`` operation will be given
        the ``--depth 1`` option. This performs a shallow clone, which avoids
        downloading the whole history and instead retrieves just the commit
        denoted by the ``GIT_TAG`` option.

      ``GIT_PROGRESS <bool>``
        .. versionadded:: 3.8

        When enabled, this option instructs the ``git clone`` operation to
        report its progress by passing it the ``--progress`` option. Without
        this option, the clone step for large projects may appear to make the
        build stall, since nothing will be logged until the clone operation
        finishes. While this option can be used to provide progress to prevent
        the appearance of the build having stalled, it may also make the build
        overly noisy if lots of external projects are used.

      ``GIT_CONFIG <option1> [<option2>...]``
        .. versionadded:: 3.8

        Specify a list of config options to pass to ``git clone``. Each option
        listed will be transformed into its own ``--config <option>`` on the
        ``git clone`` command line, with each option required to be in the
        form ``key=value``.

      ``GIT_REMOTE_UPDATE_STRATEGY <strategy>``
        .. versionadded:: 3.18

        When ``GIT_TAG`` refers to a remote branch, this option can be used to
        specify how the update step behaves.  The ``<strategy>`` must be one of
        the following:

        ``CHECKOUT``
          Ignore the local branch and always checkout the branch specified by
          ``GIT_TAG``.

        ``REBASE``
          Try to rebase the current branch to the one specified by ``GIT_TAG``.
          If there are local uncommitted changes, they will be stashed first
          and popped again after rebasing.  If rebasing or popping stashed
          changes fail, abort the rebase and halt with an error.
          When ``GIT_REMOTE_UPDATE_STRATEGY`` is not present, this is the
          default strategy unless the default has been overridden with
          ``CMAKE_EP_GIT_REMOTE_UPDATE_STRATEGY`` (see below).

        ``REBASE_CHECKOUT``
          Same as ``REBASE`` except if the rebase fails, an annotated tag will
          be created at the original ``HEAD`` position from before the rebase
          and then checkout ``GIT_TAG`` just like the ``CHECKOUT`` strategy.
          The message stored on the annotated tag will give information about
          what was attempted and the tag name will include a timestamp so that
          each failed run will add a new tag.  This strategy ensures no changes
          will be lost, but updates should always succeed if ``GIT_TAG`` refers
          to a valid ref unless there are uncommitted changes that cannot be
          popped successfully.

        The variable ``CMAKE_EP_GIT_REMOTE_UPDATE_STRATEGY`` can be set to
        override the default strategy.  This variable should not be set by a
        project, it is intended for the user to set.  It is primarily intended
        for use in continuous integration scripts to ensure that when history
        is rewritten on a remote branch, the build doesn't end up with unintended
        changes or failed builds resulting from conflicts during rebase operations.

    *Subversion*
      ``SVN_REPOSITORY <url>``
        URL of the Subversion repository.

      ``SVN_REVISION -r<rev>``
        Revision to checkout from the Subversion repository.

      ``SVN_USERNAME <username>``
        Username for the Subversion checkout and update.

      ``SVN_PASSWORD <password>``
        Password for the Subversion checkout and update.

      ``SVN_TRUST_CERT <bool>``
        Specifies whether to trust the Subversion server site certificate. If
        enabled, the ``--trust-server-cert`` option is passed to the ``svn``
        checkout and update commands.

    *Mercurial*
      ``HG_REPOSITORY <url>``
        URL of the mercurial repository.

      ``HG_TAG <tag>``
        Mercurial branch name, tag or commit id.

    *CVS*
      ``CVS_REPOSITORY <cvsroot>``
        CVSROOT of the CVS repository.

      ``CVS_MODULE <mod>``
        Module to checkout from the CVS repository.

      ``CVS_TAG <tag>``
        Tag to checkout from the CVS repository.

  **Update Step Options:**
    Whenever CMake is re-run, by default the external project's sources will be
    updated if the download method supports updates (e.g. a git repository
    would be checked if the ``GIT_TAG`` does not refer to a specific commit).

    ``UPDATE_COMMAND <cmd>...``
      Overrides the download method's update step with a custom command.
      The command may use
      :manual:`generator expressions <cmake-generator-expressions(7)>`.

    ``UPDATE_DISCONNECTED <bool>``
      .. versionadded:: 3.2

      When enabled, this option causes the update step to be skipped. It does
      not, however, prevent the download step. The update step can still be
      added as a step target (see :command:`ExternalProject_Add_StepTargets`)
      and called manually. This is useful if you want to allow developers to
      build the project when disconnected from the network (the network may
      still be needed for the download step though).

      When this option is present, it is generally advisable to make the value
      a cache variable under the developer's control rather than hard-coding
      it. If this option is not present, the default value is taken from the
      ``EP_UPDATE_DISCONNECTED`` directory property. If that is also not
      defined, updates are performed as normal. The ``EP_UPDATE_DISCONNECTED``
      directory property is intended as a convenience for controlling the
      ``UPDATE_DISCONNECTED`` behavior for an entire section of a project's
      directory hierarchy and may be a more convenient method of giving
      developers control over whether or not to perform updates (assuming the
      project also provides a cache variable or some other convenient method
      for setting the directory property).

      This may cause a step target to be created automatically for the
      ``download`` step.  See policy :policy:`CMP0114`.

  **Patch Step Options:**
    ``PATCH_COMMAND <cmd>...``
      Specifies a custom command to patch the sources after an update. By
      default, no patch command is defined. Note that it can be quite difficult
      to define an appropriate patch command that performs robustly, especially
      for download methods such as git where changing the ``GIT_TAG`` will not
      discard changes from a previous patch, but the patch command will be
      called again after updating to the new tag.

  **Configure Step Options:**
    The configure step is run after the download and update steps. By default,
    the external project is assumed to be a CMake project, but this can be
    overridden if required.

    ``CONFIGURE_COMMAND <cmd>...``
      The default configure command runs CMake with options based on the main
      project. For non-CMake external projects, the ``CONFIGURE_COMMAND``
      option must be used to override this behavior
      (:manual:`generator expressions <cmake-generator-expressions(7)>` are
      supported). For projects that require no configure step, specify this
      option with an empty string as the command to execute.

    ``CMAKE_COMMAND /.../cmake``
      Specify an alternative cmake executable for the configure step (use an
      absolute path). This is generally not recommended, since it is
      usually desirable to use the same CMake version throughout the whole
      build. This option is ignored if a custom configure command has been
      specified with ``CONFIGURE_COMMAND``.

    ``CMAKE_GENERATOR <gen>``
      Override the CMake generator used for the configure step. Without this
      option, the same generator as the main build will be used. This option is
      ignored if a custom configure command has been specified with the
      ``CONFIGURE_COMMAND`` option.

    ``CMAKE_GENERATOR_PLATFORM <platform>``
      .. versionadded:: 3.1

      Pass a generator-specific platform name to the CMake command (see
      :variable:`CMAKE_GENERATOR_PLATFORM`). It is an error to provide this
      option without the ``CMAKE_GENERATOR`` option.

    ``CMAKE_GENERATOR_TOOLSET <toolset>``
      Pass a generator-specific toolset name to the CMake command (see
      :variable:`CMAKE_GENERATOR_TOOLSET`). It is an error to provide this
      option without the ``CMAKE_GENERATOR`` option.

    ``CMAKE_GENERATOR_INSTANCE <instance>``
      .. versionadded:: 3.11

      Pass a generator-specific instance selection to the CMake command (see
      :variable:`CMAKE_GENERATOR_INSTANCE`). It is an error to provide this
      option without the ``CMAKE_GENERATOR`` option.

    ``CMAKE_ARGS <arg>...``
      The specified arguments are passed to the ``cmake`` command line. They
      can be any argument the ``cmake`` command understands, not just cache
      values defined by ``-D...`` arguments (see also
      :manual:`CMake Options <cmake(1)>`).

      .. versionadded:: 3.3
        Arguments may use :manual:`generator expressions <cmake-generator-expressions(7)>`.

    ``CMAKE_CACHE_ARGS <arg>...``
      This is an alternate way of specifying cache variables where command line
      length issues may become a problem. The arguments are expected to be in
      the form ``-Dvar:STRING=value``, which are then transformed into
      CMake :command:`set` commands with the ``FORCE`` option used. These
      ``set()`` commands are written to a pre-load script which is then applied
      using the :manual:`cmake -C <cmake(1)>` command line option.

      .. versionadded:: 3.3
        Arguments may use :manual:`generator expressions <cmake-generator-expressions(7)>`.

    ``CMAKE_CACHE_DEFAULT_ARGS <arg>...``
      .. versionadded:: 3.2

      This is the same as the ``CMAKE_CACHE_ARGS`` option except the ``set()``
      commands do not include the ``FORCE`` keyword. This means the values act
      as initial defaults only and will not override any variables already set
      from a previous run. Use this option with care, as it can lead to
      different behavior depending on whether the build starts from a fresh
      build directory or re-uses previous build contents.

      .. versionadded:: 3.15
        If the CMake generator is the ``Green Hills MULTI`` and not overridden then
        the original project's settings for the GHS toolset and target system
        customization cache variables are propagated into the external project.

    ``SOURCE_SUBDIR <dir>``
      .. versionadded:: 3.7

      When no ``CONFIGURE_COMMAND`` option is specified, the configure step
      assumes the external project has a ``CMakeLists.txt`` file at the top of
      its source tree (i.e. in ``SOURCE_DIR``). The ``SOURCE_SUBDIR`` option
      can be used to point to an alternative directory within the source tree
      to use as the top of the CMake source tree instead. This must be a
      relative path and it will be interpreted as being relative to
      ``SOURCE_DIR``.

      .. versionadded:: 3.14
        When ``BUILD_IN_SOURCE`` option is enabled, the ``BUILD_COMMAND``
        is used to point to an alternative directory within the source tree.

    ``CONFIGURE_HANDLED_BY_BUILD <bool>``
      .. versionadded:: 3.20

      Enabling this option relaxes the dependencies of the configure step on
      other external projects to order-only. This means the configure step will
      be executed after its external project dependencies are built but it will
      not be marked dirty when one of its external project dependencies is
      rebuilt. This option can be enabled when the build step is smart enough
      to figure out if the configure step needs to be rerun. CMake and Meson are
      examples of build systems whose build step is smart enough to know if the
      configure step needs to be rerun.

  **Build Step Options:**
    If the configure step assumed the external project uses CMake as its build
    system, the build step will also. Otherwise, the build step will assume a
    Makefile-based build and simply run ``make`` with no arguments as the
    default build step. This can be overridden with custom build commands if
    required.

    ``BUILD_COMMAND <cmd>...``
      Overrides the default build command
      (:manual:`generator expressions <cmake-generator-expressions(7)>` are
      supported). If this option is not given, the default build command will
      be chosen to integrate with the main build in the most appropriate way
      (e.g. using recursive ``make`` for Makefile generators or
      ``cmake --build`` if the project uses a CMake build). This option can be
      specified with an empty string as the command to make the build step do
      nothing.

    ``BUILD_IN_SOURCE <bool>``
      When this option is enabled, the build will be done directly within the
      external project's source tree. This should generally be avoided, the use
      of a separate build directory is usually preferred, but it can be useful
      when the external project assumes an in-source build. The ``BINARY_DIR``
      option should not be specified if building in-source.

    ``BUILD_ALWAYS <bool>``
      Enabling this option forces the build step to always be run. This can be
      the easiest way to robustly ensure that the external project's own build
      dependencies are evaluated rather than relying on the default
      success timestamp-based method. This option is not normally needed unless
      developers are expected to modify something the external project's build
      depends on in a way that is not detectable via the step target
      dependencies (e.g. ``SOURCE_DIR`` is used without a download method and
      developers might modify the sources in ``SOURCE_DIR``).

    ``BUILD_BYPRODUCTS <file>...``
      .. versionadded:: 3.2

      Specifies files that will be generated by the build command but which
      might or might not have their modification time updated by subsequent
      builds. These ultimately get passed through as ``BYPRODUCTS`` to the
      build step's own underlying call to :command:`add_custom_command`.

  **Install Step Options:**
    If the configure step assumed the external project uses CMake as its build
    system, the install step will also. Otherwise, the install step will assume
    a Makefile-based build and simply run ``make install`` as the default build
    step. This can be overridden with custom install commands if required.

    ``INSTALL_COMMAND <cmd>...``
      The external project's own install step is invoked as part of the main
      project's *build*. It is done after the external project's build step
      and may be before or after the external project's test step (see the
      ``TEST_BEFORE_INSTALL`` option below). The external project's install
      rules are not part of the main project's install rules, so if anything
      from the external project should be installed as part of the main build,
      these need to be specified in the main build as additional
      :command:`install` commands. The default install step builds the
      ``install`` target of the external project, but this can be overridden
      with a custom command using this option
      (:manual:`generator expressions <cmake-generator-expressions(7)>` are
      supported). Passing an empty string as the ``<cmd>`` makes the install
      step do nothing.

  **Test Step Options:**
    The test step is only defined if at least one of the following ``TEST_...``
    options are provided.

    ``TEST_COMMAND <cmd>...``
      Overrides the default test command
      (:manual:`generator expressions <cmake-generator-expressions(7)>` are
      supported). If this option is not given, the default behavior of the test
      step is to build the external project's own ``test`` target. This option
      can be specified with ``<cmd>`` as an empty string, which allows the test
      step to still be defined, but it will do nothing. Do not specify any of
      the other ``TEST_...`` options if providing an empty string as the test
      command, but prefer to omit all ``TEST_...`` options altogether if the
      test step target is not needed.

    ``TEST_BEFORE_INSTALL <bool>``
      When this option is enabled, the test step will be executed before the
      install step. The default behavior is for the test step to run after the
      install step.

    ``TEST_AFTER_INSTALL <bool>``
      This option is mainly useful as a way to indicate that the test step is
      desired but all default behavior is sufficient. Specifying this option
      with a boolean true value ensures the test step is defined and that it
      comes after the install step. If both ``TEST_BEFORE_INSTALL`` and
      ``TEST_AFTER_INSTALL`` are enabled, the latter is silently ignored.

    ``TEST_EXCLUDE_FROM_MAIN <bool>``
      .. versionadded:: 3.2

      If enabled, the main build's default ALL target will not depend on the
      test step. This can be a useful way of ensuring the test step is defined
      but only gets invoked when manually requested.
      This may cause a step target to be created automatically for either
      the ``install`` or ``build`` step.  See policy :policy:`CMP0114`.

  **Output Logging Options:**
    Each of the following ``LOG_...`` options can be used to wrap the relevant
    step in a script to capture its output to files. The log files will be
    created in ``LOG_DIR`` if supplied or otherwise the ``STAMP_DIR``
    directory with step-specific file names.

    ``LOG_DOWNLOAD <bool>``
      When enabled, the output of the download step is logged to files.

    ``LOG_UPDATE <bool>``
      When enabled, the output of the update step is logged to files.

    ``LOG_PATCH <bool>``
      .. versionadded:: 3.14

      When enabled, the output of the patch step is logged to files.

    ``LOG_CONFIGURE <bool>``
      When enabled, the output of the configure step is logged to files.

    ``LOG_BUILD <bool>``
      When enabled, the output of the build step is logged to files.

    ``LOG_INSTALL <bool>``
      When enabled, the output of the install step is logged to files.

    ``LOG_TEST <bool>``
      When enabled, the output of the test step is logged to files.

    ``LOG_MERGED_STDOUTERR <bool>``
      .. versionadded:: 3.14

      When enabled, stdout and stderr will be merged for any step whose
      output is being logged to files.

    ``LOG_OUTPUT_ON_FAILURE <bool>``
      .. versionadded:: 3.14

      This option only has an effect if at least one of the other ``LOG_<step>``
      options is enabled.  If an error occurs for a step which has logging to
      file enabled, that step's output will be printed to the console if
      ``LOG_OUTPUT_ON_FAILURE`` is set to true.  For cases where a large amount
      of output is recorded, just the end of that output may be printed to the
      console.

  **Terminal Access Options:**
    .. versionadded:: 3.4

    Steps can be given direct access to the terminal in some cases. Giving a
    step access to the terminal may allow it to receive terminal input if
    required, such as for authentication details not provided by other options.
    With the :generator:`Ninja` generator, these options place the steps in the
    ``console`` :prop_gbl:`job pool <JOB_POOLS>`. Each step can be given access
    to the terminal individually via the following options:

    ``USES_TERMINAL_DOWNLOAD <bool>``
      Give the download step access to the terminal.

    ``USES_TERMINAL_UPDATE <bool>``
      Give the update step access to the terminal.

    ``USES_TERMINAL_PATCH <bool>``
      .. versionadded:: 3.20

      Give the patch step access to the terminal.

    ``USES_TERMINAL_CONFIGURE <bool>``
      Give the configure step access to the terminal.

    ``USES_TERMINAL_BUILD <bool>``
      Give the build step access to the terminal.

    ``USES_TERMINAL_INSTALL <bool>``
      Give the install step access to the terminal.

    ``USES_TERMINAL_TEST <bool>``
      Give the test step access to the terminal.

  **Target Options:**
    ``DEPENDS <targets>...``
      Specify other targets on which the external project depends. The other
      targets will be brought up to date before any of the external project's
      steps are executed. Because the external project uses additional custom
      targets internally for each step, the ``DEPENDS`` option is the most
      convenient way to ensure all of those steps depend on the other targets.
      Simply doing
      :command:`add_dependencies(\<name\> \<targets\>) <add_dependencies>` will
      not make any of the steps dependent on ``<targets>``.

    ``EXCLUDE_FROM_ALL <bool>``
      When enabled, this option excludes the external project from the default
      ALL target of the main build.

    ``STEP_TARGETS <step-target>...``
      Generate custom targets for the specified steps. This is required if the
      steps need to be triggered manually or if they need to be used as
      dependencies of other targets. If this option is not specified, the
      default value is taken from the ``EP_STEP_TARGETS`` directory property.
      See :command:`ExternalProject_Add_StepTargets` below for further
      discussion of the effects of this option.

    ``INDEPENDENT_STEP_TARGETS <step-target>...``
      .. deprecated:: 3.19
        This is allowed only if policy :policy:`CMP0114` is not set to ``NEW``.

      Generates custom targets for the specified steps and prevent these targets
      from having the usual dependencies applied to them. If this option is not
      specified, the default value is taken from the
      ``EP_INDEPENDENT_STEP_TARGETS`` directory property. This option is mostly
      useful for allowing individual steps to be driven independently, such as
      for a CDash setup where each step should be initiated and reported
      individually rather than as one whole build. See
      :command:`ExternalProject_Add_StepTargets` below for further discussion
      of the effects of this option.

  **Miscellaneous Options:**
    ``LIST_SEPARATOR <sep>``
      For any of the various ``..._COMMAND`` options, replace ``;`` with
      ``<sep>`` in the specified command lines. This can be useful where list
      variables may be given in commands where they should end up as
      space-separated arguments (``<sep>`` would be a single space character
      string in this case).

    ``COMMAND <cmd>...``
      Any of the other ``..._COMMAND`` options can have additional commands
      appended to them by following them with as many ``COMMAND ...`` options
      as needed
      (:manual:`generator expressions <cmake-generator-expressions(7)>` are
      supported). For example:

      .. code-block:: cmake

        ExternalProject_Add(example
          ... # Download options, etc.
          BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Starting $<CONFIG> build"
          COMMAND       ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
          COMMAND       ${CMAKE_COMMAND} -E echo "$<CONFIG> build complete"
        )

  It should also be noted that each build step is created via a call to
  :command:`ExternalProject_Add_Step`. See that command's documentation for the
  automatic substitutions that are supported for some options.

Obtaining Project Properties
""""""""""""""""""""""""""""

.. command:: ExternalProject_Get_Property

  The ``ExternalProject_Get_Property()`` function retrieves external project
  target properties:

  .. code-block:: cmake

    ExternalProject_Get_Property(<name> <prop1> [<prop2>...])

  The function stores property values in variables of the same name. Property
  names correspond to the keyword argument names of ``ExternalProject_Add()``.
  For example, the source directory might be retrieved like so:

  .. code-block:: cmake

    ExternalProject_Get_property(myExtProj SOURCE_DIR)
    message("Source dir of myExtProj = ${SOURCE_DIR}")

Explicit Step Management
""""""""""""""""""""""""

The ``ExternalProject_Add()`` function on its own is often sufficient for
incorporating an external project into the main build. Certain scenarios
require additional work to implement desired behavior, such as adding in a
custom step or making steps available as manually triggerable targets. The
``ExternalProject_Add_Step()``, ``ExternalProject_Add_StepTargets()`` and
``ExternalProject_Add_StepDependencies`` functions provide the lower level
control needed to implement such step-level capabilities.

.. command:: ExternalProject_Add_Step

  The ``ExternalProject_Add_Step()`` function specifies an additional custom
  step for an external project defined by an earlier call to
  :command:`ExternalProject_Add`:

  .. code-block:: cmake

    ExternalProject_Add_Step(<name> <step> [<option>...])

  ``<name>`` is the same as the name passed to the original call to
  :command:`ExternalProject_Add`. The specified ``<step>`` must not be one of
  the pre-defined steps (``mkdir``, ``download``, ``update``,
  ``patch``, ``configure``, ``build``, ``install`` or ``test``). The supported
  options are:

  ``COMMAND <cmd>...``
    The command line to be executed by this custom step
    (:manual:`generator expressions <cmake-generator-expressions(7)>` are
    supported). This option can be repeated multiple times to specify multiple
    commands to be executed in order.

  ``COMMENT "<text>..."``
    Text to be printed when the custom step executes.

  ``DEPENDEES <step>...``
    Other steps (custom or pre-defined) on which this step depends.

  ``DEPENDERS <step>...``
    Other steps (custom or pre-defined) that depend on this new custom step.

  ``DEPENDS <file>...``
    Files on which this custom step depends.

  ``INDEPENDENT <bool>``
    .. versionadded:: 3.19

    Specifies whether this step is independent of the external dependencies
    specified by the :command:`ExternalProject_Add`'s ``DEPENDS`` option.
    The default is ``FALSE``.  Steps marked as independent may depend only
    on other steps marked independent.  See policy :policy:`CMP0114`.

    Note that this use of the term "independent" refers only to independence
    from external targets specified by the ``DEPENDS`` option and is
    orthogonal to a step's dependencies on other steps.

    If a step target is created for an independent step by the
    :command:`ExternalProject_Add` ``STEP_TARGETS`` option or by the
    :command:`ExternalProject_Add_StepTargets` function, it will not depend
    on the external targets, but may depend on targets for other steps.

  ``BYPRODUCTS <file>...``
    .. versionadded:: 3.2

    Files that will be generated by this custom step but which might or might
    not have their modification time updated by subsequent builds. This list of
    files will ultimately be passed through as the ``BYPRODUCTS`` option to the
    :command:`add_custom_command` used to implement the custom step internally.

  ``ALWAYS <bool>``
    When enabled, this option specifies that the custom step should always be
    run (i.e. that it is always considered out of date).

  ``EXCLUDE_FROM_MAIN <bool>``
    When enabled, this option specifies that the external project's main target
    does not depend on the custom step.
    This may cause step targets to be created automatically for the steps on
    which this step depends.  See policy :policy:`CMP0114`.

  ``WORKING_DIRECTORY <dir>``
    Specifies the working directory to set before running the custom step's
    command. If this option is not specified, the directory will be the value
    of the :variable:`CMAKE_CURRENT_BINARY_DIR` at the point where
    ``ExternalProject_Add_Step()`` was called.

  ``LOG <bool>``
    If set, this causes the output from the custom step to be captured to files
    in the external project's ``LOG_DIR`` if supplied or ``STAMP_DIR``.

  ``USES_TERMINAL <bool>``
    If enabled, this gives the custom step direct access to the terminal if
    possible.

  The command line, comment, working directory and byproducts of every
  standard and custom step are processed to replace the tokens
  ``<SOURCE_DIR>``, ``<SOURCE_SUBDIR>``, ``<BINARY_DIR>``, ``<INSTALL_DIR>``
  ``<TMP_DIR>``, ``<DOWNLOAD_DIR>`` and ``<DOWNLOADED_FILE>`` with their
  corresponding property values defined in the original call to
  :command:`ExternalProject_Add`.

  .. versionadded:: 3.3
    Token replacement is extended to byproducts.

  .. versionadded:: 3.11
    The ``<DOWNLOAD_DIR>`` substitution token.

.. command:: ExternalProject_Add_StepTargets

  The ``ExternalProject_Add_StepTargets()`` function generates targets for the
  steps listed. The name of each created target will be of the form
  ``<name>-<step>``:

  .. code-block:: cmake

    ExternalProject_Add_StepTargets(<name> <step1> [<step2>...])

  Creating a target for a step allows it to be used as a dependency of another
  target or to be triggered manually. Having targets for specific steps also
  allows them to be driven independently of each other by specifying targets on
  build command lines. For example, you may be submitting to a sub-project
  based dashboard where you want to drive the configure portion of the build,
  then submit to the dashboard, followed by the build portion, followed
  by tests. If you invoke a custom target that depends on a step halfway
  through the step dependency chain, then all the previous steps will also run
  to ensure everything is up to date.

  Internally, :command:`ExternalProject_Add` calls
  :command:`ExternalProject_Add_Step` to create each step. If any
  ``STEP_TARGETS`` were specified, then ``ExternalProject_Add_StepTargets()``
  will also be called after :command:`ExternalProject_Add_Step`.  Even if a
  step is not mentioned in the ``STEP_TARGETS`` option,
  ``ExternalProject_Add_StepTargets()`` can still be called later to manually
  define a target for the step.

  The ``STEP_TARGETS`` option for :command:`ExternalProject_Add` is generally
  the easiest way to ensure targets are created for specific steps of interest.
  For custom steps, ``ExternalProject_Add_StepTargets()`` must be called
  explicitly if a target should also be created for that custom step.
  An alternative to these two options is to populate the ``EP_STEP_TARGETS``
  directory property.  It acts as a default for the step target options and
  can save having to repeatedly specify the same set of step targets when
  multiple external projects are being defined.

  .. versionadded:: 3.19
    If :policy:`CMP0114` is set to ``NEW``, step targets are fully responsible
    for holding the custom commands implementing their steps.  The primary target
    created by ``ExternalProject_Add`` depends on the step targets, and the
    step targets depend on each other.  The target-level dependencies match
    the file-level dependencies used by the custom commands for each step.
    The targets for steps created with :command:`ExternalProject_Add_Step`'s
    ``INDEPENDENT`` option do not depend on the external targets specified
    by :command:`ExternalProject_Add`'s ``DEPENDS`` option.  The predefined
    steps ``mkdir``, ``download``, ``update``, and ``patch`` are independent.

  If :policy:`CMP0114` is not ``NEW``, the following deprecated behavior
  is available:

  * A deprecated ``NO_DEPENDS`` option may be specified immediately after the
    ``<name>`` and before the first step.
    If the ``NO_DEPENDS`` option is specified, the step target will not depend on
    the dependencies of the external project (i.e. on any dependencies of the
    ``<name>`` custom target created by :command:`ExternalProject_Add`). This is
    usually safe for the ``download``, ``update`` and ``patch`` steps, since they
    do not typically require that the dependencies are updated and built. Using
    ``NO_DEPENDS`` for any of the other pre-defined steps, however, may break
    parallel builds. Only use ``NO_DEPENDS`` where it is certain that the named
    steps genuinely do not have dependencies. For custom steps, consider whether
    or not the custom commands require the dependencies to be configured, built
    and installed.

  * The ``INDEPENDENT_STEP_TARGETS`` option for :command:`ExternalProject_Add`,
    or the ``EP_INDEPENDENT_STEP_TARGETS`` directory property, tells the
    function to call ``ExternalProject_Add_StepTargets()`` internally
    using the ``NO_DEPENDS`` option for the specified steps.

.. command:: ExternalProject_Add_StepDependencies

  .. versionadded:: 3.2

  The ``ExternalProject_Add_StepDependencies()`` function can be used to add
  dependencies to a step. The dependencies added must be targets CMake already
  knows about (these can be ordinary executable or library targets, custom
  targets or even step targets of another external project):

  .. code-block:: cmake

    ExternalProject_Add_StepDependencies(<name> <step> <target1> [<target2>...])

  This function takes care to set both target and file level dependencies and
  will ensure that parallel builds will not break. It should be used instead of
  :command:`add_dependencies` whenever adding a dependency for some of the step
  targets generated by the ``ExternalProject`` module.

Examples
^^^^^^^^

The following example shows how to download and build a hypothetical project
called *FooBar* from github:

.. code-block:: cmake

  include(ExternalProject)
  ExternalProject_Add(foobar
    GIT_REPOSITORY    git@github.com:FooCo/FooBar.git
    GIT_TAG           origin/release/1.2.3
  )

For the sake of the example, also define a second hypothetical external project
called *SecretSauce*, which is downloaded from a web server. Two URLs are given
to take advantage of a faster internal network if available, with a fallback to
a slower external server. The project is a typical ``Makefile`` project with no
configure step, so some of the default commands are overridden. The build is
only required to build the *sauce* target:

.. code-block:: cmake

  find_program(MAKE_EXE NAMES gmake nmake make)
  ExternalProject_Add(secretsauce
    URL               http://intranet.somecompany.com/artifacts/sauce-2.7.tgz
                      https://www.somecompany.com/downloads/sauce-2.7.zip
    URL_HASH          MD5=d41d8cd98f00b204e9800998ecf8427e
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ${MAKE_EXE} sauce
  )

Suppose the build step of ``secretsauce`` requires that ``foobar`` must already
be built. This could be enforced like so:

.. code-block:: cmake

  ExternalProject_Add_StepDependencies(secretsauce build foobar)

Another alternative would be to create a custom target for ``foobar``'s build
step and make ``secretsauce`` depend on that rather than the whole ``foobar``
project. This would mean ``foobar`` only needs to be built, it doesn't need to
run its install or test steps before ``secretsauce`` can be built. The
dependency can also be defined along with the ``secretsauce`` project:

.. code-block:: cmake

  ExternalProject_Add_StepTargets(foobar build)
  ExternalProject_Add(secretsauce
    URL               http://intranet.somecompany.com/artifacts/sauce-2.7.tgz
                      https://www.somecompany.com/downloads/sauce-2.7.zip
    URL_HASH          MD5=d41d8cd98f00b204e9800998ecf8427e
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ${MAKE_EXE} sauce
    DEPENDS           foobar-build
  )

Instead of calling :command:`ExternalProject_Add_StepTargets`, the target could
be defined along with the ``foobar`` project itself:

.. code-block:: cmake

  ExternalProject_Add(foobar
    GIT_REPOSITORY git@github.com:FooCo/FooBar.git
    GIT_TAG        origin/release/1.2.3
    STEP_TARGETS   build
  )

If many external projects should have the same set of step targets, setting a
directory property may be more convenient. The ``build`` step target could be
created automatically by setting the ``EP_STEP_TARGETS`` directory property
before creating the external projects with :command:`ExternalProject_Add`:

.. code-block:: cmake

  set_property(DIRECTORY PROPERTY EP_STEP_TARGETS build)

Lastly, suppose that ``secretsauce`` provides a script called ``makedoc`` which
can be used to generate its own documentation. Further suppose that the script
expects the output directory to be provided as the only parameter and that it
should be run from the ``secretsauce`` source directory. A custom step and a
custom target to trigger the script can be defined like so:

.. code-block:: cmake

  ExternalProject_Add_Step(secretsauce docs
    COMMAND           <SOURCE_DIR>/makedoc <BINARY_DIR>
    WORKING_DIRECTORY <SOURCE_DIR>
    COMMENT           "Building secretsauce docs"
    ALWAYS            TRUE
    EXCLUDE_FROM_MAIN TRUE
  )
  ExternalProject_Add_StepTargets(secretsauce docs)

The custom step could then be triggered from the main build like so::

  cmake --build . --target secretsauce-docs

#]=======================================================================]

cmake_policy(PUSH)
cmake_policy(SET CMP0054 NEW) # if() quoted variables not dereferenced
cmake_policy(SET CMP0057 NEW) # if() supports IN_LIST

macro(_ep_get_hash_algos out_var)
  set(${out_var}
    MD5
    SHA1
    SHA224
    SHA256
    SHA384
    SHA512
    SHA3_224
    SHA3_256
    SHA3_384
    SHA3_512
  )
endmacro()

macro(_ep_get_hash_regex out_var)
  _ep_get_hash_algos(${out_var})
  list(JOIN ${out_var} "|" ${out_var})
  set(${out_var} "^(${${out_var}})=([0-9A-Fa-f]+)$")
endmacro()

function(_ep_parse_arguments_to_vars keywords name ns args)
  # Transfer the arguments into variables in the calling scope.
  # Because some keywords can be repeated, we can't use cmake_parse_arguments().
  # Instead, we loop through the args and consider the namespace starting with
  # an upper-case letter followed by at least two more upper-case letters,
  # numbers or underscores to be keywords.

  foreach(key IN LISTS keywords)
    unset(${ns}${key})
  endforeach()

  set(key)

  foreach(arg IN LISTS args)
    set(is_value 1)

    if(arg MATCHES "^[A-Z][A-Z0-9_][A-Z0-9_]+$" AND
        NOT (("x${arg}x" STREQUAL "x${key}x") AND ("x${key}x" STREQUAL "xCOMMANDx")) AND
        NOT arg MATCHES "^(TRUE|FALSE)$")
      if(arg IN_LIST keywords)
        set(is_value 0)
      endif()
    endif()

    if(is_value)
      if(key)
        # Value
        list(APPEND ${ns}${key} "${arg}")
      else()
        # Missing Keyword
        message(AUTHOR_WARNING "value '${arg}' with no previous keyword")
      endif()
    else()
      set(key "${arg}")
    endif()
  endforeach()

  foreach(key IN LISTS keywords)
    if(DEFINED ${ns}${key})
      set(${ns}${key} "${${ns}${key}}" PARENT_SCOPE)
    else()
      unset(${ns}${key} PARENT_SCOPE)
    endif()
  endforeach()

endfunction()

function(_ep_parse_arguments keywords name ns args)
  _ep_parse_arguments_to_vars("${keywords}" ${name} ${ns} "${args}")

  # Transfer the arguments to the target as target properties. These are
  # read by the various steps, potentially from different scopes.
  foreach(key IN LISTS keywords)
    if(DEFINED ${ns}${key})
      set_property(TARGET ${name} PROPERTY ${ns}${key} "${${ns}${key}}")
    endif()
  endforeach()

endfunction()

if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
  define_property(DIRECTORY PROPERTY "EP_BASE" INHERITED
    BRIEF_DOCS "Base directory for External Project storage."
    FULL_DOCS
    "See documentation of the ExternalProject_Add() function in the "
    "ExternalProject module."
    )

  define_property(DIRECTORY PROPERTY "EP_PREFIX" INHERITED
    BRIEF_DOCS "Top prefix for External Project storage."
    FULL_DOCS
    "See documentation of the ExternalProject_Add() function in the "
    "ExternalProject module."
    )

  define_property(DIRECTORY PROPERTY "EP_STEP_TARGETS" INHERITED
    BRIEF_DOCS
    "List of ExternalProject steps that automatically get corresponding targets"
    FULL_DOCS
    "These targets will be dependent on the main target dependencies. "
    "See documentation of the ExternalProject_Add_StepTargets() function in the "
    "ExternalProject module."
    )

  define_property(DIRECTORY PROPERTY "EP_INDEPENDENT_STEP_TARGETS" INHERITED
    BRIEF_DOCS
    "List of ExternalProject steps that automatically get corresponding targets"
    FULL_DOCS
    "These targets will not be dependent on the main target dependencies. "
    "See documentation of the ExternalProject_Add_StepTargets() function in the "
    "ExternalProject module."
    )

  define_property(DIRECTORY PROPERTY "EP_UPDATE_DISCONNECTED" INHERITED
    BRIEF_DOCS "Never update automatically from the remote repo."
    FULL_DOCS
    "See documentation of the ExternalProject_Add() function in the "
    "ExternalProject module."
    )
endif()

function(_ep_write_gitclone_script
         script_filename
         source_dir
         git_EXECUTABLE
         git_repository
         git_tag
         git_remote_name
         init_submodules
         git_submodules_recurse
         git_submodules
         git_shallow
         git_progress
         git_config
         src_name
         work_dir
         gitclone_infofile
         gitclone_stampfile
         tls_verify
         quiet)

  if(NOT GIT_VERSION_STRING VERSION_LESS 1.8.5)
    # Use `git checkout <tree-ish> --` to avoid ambiguity with a local path.
    set(git_checkout_explicit-- "--")
  else()
    # Use `git checkout <branch>` even though this risks ambiguity with a
    # local path.  Unfortunately we cannot use `git checkout <tree-ish> --`
    # because that will not search for remote branch names, a common use case.
    set(git_checkout_explicit-- "")
  endif()
  if("${git_tag}" STREQUAL "")
    message(FATAL_ERROR "Tag for git checkout should not be empty.")
  endif()

  if(GIT_VERSION_STRING VERSION_LESS 2.20 OR 2.21 VERSION_LESS_EQUAL GIT_VERSION_STRING)
    set(git_clone_options "--no-checkout")
  else()
    set(git_clone_options)
  endif()
  if(git_shallow)
    if(NOT GIT_VERSION_STRING VERSION_LESS 1.7.10)
      list(APPEND git_clone_options "--depth 1 --no-single-branch")
    else()
      list(APPEND git_clone_options "--depth 1")
    endif()
  endif()
  if(git_progress)
    list(APPEND git_clone_options --progress)
  endif()
  foreach(config IN LISTS git_config)
    list(APPEND git_clone_options --config \"${config}\")
  endforeach()
  if(NOT ${git_remote_name} STREQUAL "origin")
    list(APPEND git_clone_options --origin \"${git_remote_name}\")
  endif()

  string (REPLACE ";" " " git_clone_options "${git_clone_options}")

  set(git_options)
  # disable cert checking if explicitly told not to do it
  if(NOT "x${tls_verify}" STREQUAL "x" AND NOT tls_verify)
    set(git_options
      -c http.sslVerify=false)
  endif()
  string (REPLACE ";" " " git_options "${git_options}")

  configure_file(
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/gitclone.cmake.in
    ${script_filename}
    @ONLY
  )

endfunction()

function(_ep_write_hgclone_script
         script_filename
         source_dir
         hg_EXECUTABLE
         hg_repository
         hg_tag
         src_name
         work_dir
         hgclone_infofile
         hgclone_stampfile
         quiet)

  if("${hg_tag}" STREQUAL "")
    message(FATAL_ERROR "Tag for hg checkout should not be empty.")
  endif()

  configure_file(
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/hgclone.cmake.in
    ${script_filename}
    @ONLY
  )

endfunction()


function(_ep_write_gitupdate_script
         script_filename
         git_EXECUTABLE
         git_tag
         git_remote_name
         init_submodules
         git_submodules_recurse
         git_submodules
         git_repository
         work_dir
         git_update_strategy
         quiet)

  if("${git_tag}" STREQUAL "")
    message(FATAL_ERROR "Tag for git checkout should not be empty.")
  endif()
  set(git_stash_save_options --quiet)
  if(GIT_VERSION_STRING VERSION_GREATER_EQUAL 1.7.7)
    # This avoids stashing files covered by .gitignore
    list(APPEND git_stash_save_options --include-untracked)
  elseif(GIT_VERSION_STRING VERSION_GREATER_EQUAL 1.7.6)
    # Untracked files, but also ignored files, so potentially slower
    list(APPEND git_stash_save_options --all)
  endif()

  configure_file(
    "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/gitupdate.cmake.in"
    "${script_filename}"
    @ONLY
  )
endfunction()

function(_ep_write_hgupdate_script
         script_filename
         hg_EXECUTABLE
         hg_tag
         work_dir
         quiet)

  configure_file(
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/hgupdate.cmake.in
    ${script_filename}
    @ONLY
  )

endfunction()

function(_ep_write_copydir_script
         script_filename
         from_dir
         to_dir)

  configure_file(
    "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/copydir.cmake.in"
    "${script_filename}"
    @ONLY
  )
endfunction()

function(_ep_write_downloadfile_script
         script_filename
         REMOTE
         LOCAL
         timeout
         inactivity_timeout
         no_progress
         hash
         tls_verify
         tls_cainfo
         userpwd
         http_headers
         netrc
         netrc_file
         extract_script_filename
         quiet)

  if(timeout)
    set(TIMEOUT_ARGS TIMEOUT ${timeout})
    set(TIMEOUT_MSG "${timeout} seconds")
  else()
    set(TIMEOUT_ARGS "# no TIMEOUT")
    set(TIMEOUT_MSG "none")
  endif()
  if(inactivity_timeout)
    set(INACTIVITY_TIMEOUT_ARGS INACTIVITY_TIMEOUT ${inactivity_timeout})
    set(INACTIVITY_TIMEOUT_MSG "${inactivity_timeout} seconds")
  else()
    set(INACTIVITY_TIMEOUT_ARGS "# no INACTIVITY_TIMEOUT")
    set(INACTIVITY_TIMEOUT_MSG "none")
  endif()


  if(no_progress OR quiet)
    set(SHOW_PROGRESS "")
  else()
    set(SHOW_PROGRESS "SHOW_PROGRESS")
  endif()

  _ep_get_hash_regex(_ep_hash_regex)
  if("${hash}" MATCHES "${_ep_hash_regex}")
    set(ALGO "${CMAKE_MATCH_1}")
    string(TOLOWER "${CMAKE_MATCH_2}" EXPECT_VALUE)
  else()
    set(ALGO "")
    set(EXPECT_VALUE "")
  endif()

  set(TLS_VERIFY_CODE "")
  set(TLS_CAINFO_CODE "")
  set(NETRC_CODE "")
  set(NETRC_FILE_CODE "")

  # check for curl globals in the project
  if(DEFINED CMAKE_TLS_VERIFY)
    set(TLS_VERIFY_CODE "set(CMAKE_TLS_VERIFY ${CMAKE_TLS_VERIFY})")
  endif()
  if(DEFINED CMAKE_TLS_CAINFO)
    set(TLS_CAINFO_CODE "set(CMAKE_TLS_CAINFO \"${CMAKE_TLS_CAINFO}\")")
  endif()
  if(DEFINED CMAKE_NETRC)
    set(NETRC_CODE "set(CMAKE_NETRC \"${CMAKE_NETRC}\")")
  endif()
  if(DEFINED CMAKE_NETRC_FILE)
    set(NETRC_FILE_CODE "set(CMAKE_NETRC_FILE \"${CMAKE_NETRC_FILE}\")")
  endif()

  # now check for curl locals so that the local values
  # will override the globals

  # check for tls_verify argument
  string(LENGTH "${tls_verify}" tls_verify_len)
  if(tls_verify_len GREATER 0)
    set(TLS_VERIFY_CODE "set(CMAKE_TLS_VERIFY ${tls_verify})")
  endif()
  # check for tls_cainfo argument
  string(LENGTH "${tls_cainfo}" tls_cainfo_len)
  if(tls_cainfo_len GREATER 0)
    set(TLS_CAINFO_CODE "set(CMAKE_TLS_CAINFO \"${tls_cainfo}\")")
  endif()
  # check for netrc argument
  string(LENGTH "${netrc}" netrc_len)
  if(netrc_len GREATER 0)
    set(NETRC_CODE "set(CMAKE_NETRC \"${netrc}\")")
  endif()
  # check for netrc_file argument
  string(LENGTH "${netrc_file}" netrc_file_len)
  if(netrc_file_len GREATER 0)
    set(NETRC_FILE_CODE "set(CMAKE_NETRC_FILE \"${netrc_file}\")")
  endif()

  if(userpwd STREQUAL ":")
    set(USERPWD_ARGS)
  else()
    set(USERPWD_ARGS USERPWD "${userpwd}")
  endif()

  set(HTTP_HEADERS_ARGS "")
  if(NOT http_headers STREQUAL "")
    foreach(header ${http_headers})
      set(
          HTTP_HEADERS_ARGS
          "HTTPHEADER \"${header}\"\n        ${HTTP_HEADERS_ARGS}"
      )
    endforeach()
  endif()

  # Used variables:
  # * TLS_VERIFY_CODE
  # * TLS_CAINFO_CODE
  # * ALGO
  # * EXPECT_VALUE
  # * REMOTE
  # * LOCAL
  # * SHOW_PROGRESS
  # * TIMEOUT_ARGS
  # * TIMEOUT_MSG
  # * USERPWD_ARGS
  # * HTTP_HEADERS_ARGS
  configure_file(
    "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/download.cmake.in"
    "${script_filename}"
    @ONLY
  )
endfunction()

function(_ep_write_verifyfile_script
         script_filename
         LOCAL
         hash
         extract_script_filename
         quiet)

  _ep_get_hash_regex(_ep_hash_regex)
  if("${hash}" MATCHES "${_ep_hash_regex}")
    set(ALGO "${CMAKE_MATCH_1}")
    string(TOLOWER "${CMAKE_MATCH_2}" EXPECT_VALUE)
  else()
    set(ALGO "")
    set(EXPECT_VALUE "")
  endif()

  # Used variables:
  # * ALGO
  # * EXPECT_VALUE
  # * LOCAL
  # * extract_script_filename
  configure_file(
    "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/verify.cmake.in"
    "${script_filename}"
    @ONLY
  )
endfunction()


function(_ep_write_extractfile_script
         script_filename
         name
         filename
         directory
         quiet)

  set(args "")

  if(filename MATCHES "(\\.|=)(7z|tar\\.bz2|tar\\.gz|tar\\.xz|tbz2|tgz|txz|zip)$")
    set(args xfz)
  endif()

  if(filename MATCHES "(\\.|=)tar$")
    set(args xf)
  endif()

  if(args STREQUAL "")
    message(FATAL_ERROR
      "Do not know how to extract '${filename}' -- known types are: "
      ".7z, .tar, .tar.bz2, .tar.gz, .tar.xz, .tbz2, .tgz, .txz and .zip")
  endif()

  configure_file(
    "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/extractfile.cmake.in"
    "${script_filename}"
    @ONLY
  )

endfunction()


# This function is an implementation detail of ExternalProject_Add() and
# _ep_do_preconfigure_steps_now().
#
# The function expects keyword arguments to have already been parsed into
# variables of the form _EP_<keyword>. It will create the various directories
# before returning and it will populate variables of the form
# _EP_<location>_DIR in the calling scope.
#
# Variables will also be set in the calling scope to enable subsequently
# calling _ep_add_preconfigure_command() for the mkdir step.
#
function(_ep_prepare_directories name)

  set(prefix ${_EP_PREFIX})
  if(NOT prefix)
    get_property(prefix DIRECTORY PROPERTY EP_PREFIX)
    if(NOT prefix)
      get_property(base DIRECTORY PROPERTY EP_BASE)
      if(NOT base)
        set(prefix "${name}-prefix")
      endif()
    endif()
  endif()
  if(prefix)
    file(TO_CMAKE_PATH "${prefix}" prefix)
    set(tmp_default "${prefix}/tmp")
    set(download_default "${prefix}/src")
    set(source_default "${prefix}/src/${name}")
    set(binary_default "${prefix}/src/${name}-build")
    set(stamp_default "${prefix}/src/${name}-stamp")
    set(install_default "${prefix}")
  else()
    file(TO_CMAKE_PATH "${base}" base)
    set(tmp_default "${base}/tmp/${name}")
    set(download_default "${base}/Download/${name}")
    set(source_default "${base}/Source/${name}")
    set(binary_default "${base}/Build/${name}")
    set(stamp_default "${base}/Stamp/${name}")
    set(install_default "${base}/Install/${name}")
  endif()

  set(build_in_source "${_EP_BUILD_IN_SOURCE}")
  if(build_in_source)
    if(DEFINED _EP_BINARY_DIR)
      message(FATAL_ERROR
        "External project ${name} has both BINARY_DIR and BUILD_IN_SOURCE!")
    endif()
  endif()
  set(top "${CMAKE_CURRENT_BINARY_DIR}")

  # Apply defaults and convert to absolute paths.
  set(places stamp download source binary install tmp)
  foreach(var ${places})
    string(TOUPPER "${var}" VAR)
    set(${var}_dir "${_EP_${VAR}_DIR}")
    if(NOT ${var}_dir)
      set(${var}_dir "${${var}_default}")
    endif()
    if(NOT IS_ABSOLUTE "${${var}_dir}")
      get_filename_component(${var}_dir "${top}/${${var}_dir}" ABSOLUTE)
    endif()
    file(TO_CMAKE_PATH "${${var}_dir}" ${var}_dir)
  endforeach()

  # Special case for default log directory based on stamp directory.
  set(log_dir "${_EP_LOG_DIR}")
  if(NOT log_dir)
    set(log_dir "${stamp_dir}")
  else()
    if(NOT IS_ABSOLUTE "${log_dir}")
      get_filename_component(log_dir "${top}/${log_dir}" ABSOLUTE)
    endif()
  endif()
  file(TO_CMAKE_PATH "${log_dir}" log_dir)
  list(APPEND places log)

  set(source_subdir "${_EP_SOURCE_SUBDIR}")
  if(source_subdir)
    if(IS_ABSOLUTE "${source_subdir}")
      message(FATAL_ERROR
        "External project ${name} has non-relative SOURCE_SUBDIR!")
    endif()
    string(REPLACE "\\" "/" source_subdir "${source_subdir}")
    # Prefix with a slash so that when appended to the source directory, it
    # behaves as expected.
    string(PREPEND source_subdir "/")
  endif()

  if(build_in_source)
    set(binary_dir "${source_dir}${source_subdir}")
  endif()

  # This script will be used both here and by the mkdir step. We create the
  # directories now at configure time and ensure they exists again at build
  # time (since somebody might remove one of the required directories and try
  # to rebuild without re-running cmake). They need to exist now at makefile
  # generation time for Borland make and wmake so that CMake may generate
  # makefiles with "cd C:\short\paths\with\no\spaces" commands in them.
  set(script_filename "${tmp_dir}/${name}-mkdirs.cmake")
  configure_file(
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/mkdirs.cmake.in
    ${script_filename}
    @ONLY
  )
  include(${script_filename})

  set(comment "Creating directories for '${name}'")
  set(cmd ${CMAKE_COMMAND} -P ${script_filename})

  # Provide variables that can be used later to create a custom command or
  # invoke the step directly
  set(_EPcomment_MKDIR         "${comment}" PARENT_SCOPE)
  set(_EPcommand_MKDIR         "${cmd}"     PARENT_SCOPE)
  set(_EPalways_MKDIR          FALSE        PARENT_SCOPE)
  set(_EPexcludefrommain_MKDIR FALSE        PARENT_SCOPE)
  set(_EPdepends_MKDIR         ""           PARENT_SCOPE)
  set(_EPdependees_MKDIR       ""           PARENT_SCOPE)

  foreach(var ${places})
    string(TOUPPER "${var}" VAR)
    set(_EP_${VAR}_DIR "${${var}_dir}" PARENT_SCOPE)
  endforeach()
  set(_EP_SOURCE_SUBDIR "${source_subdir}" PARENT_SCOPE)

endfunction()

# IMPORTANT: this MUST be a macro and not a function because of the
# in-place replacements that occur in each ${var}
#
macro(_ep_replace_location_tags target_name)
  set(vars ${ARGN})
  foreach(var ${vars})
    if(${var})
      foreach(dir SOURCE_DIR SOURCE_SUBDIR BINARY_DIR INSTALL_DIR TMP_DIR DOWNLOAD_DIR DOWNLOADED_FILE LOG_DIR)
        get_property(val TARGET ${target_name} PROPERTY _EP_${dir})
        string(REPLACE "<${dir}>" "${val}" ${var} "${${var}}")
      endforeach()
    endif()
  endforeach()
endmacro()

macro(_ep_replace_location_tags_from_vars)
  set(vars ${ARGN})
  foreach(var ${vars})
    if(${var})
      foreach(dir SOURCE_DIR SOURCE_SUBDIR BINARY_DIR INSTALL_DIR TMP_DIR DOWNLOAD_DIR DOWNLOADED_FILE LOG_DIR)
        string(REPLACE "<${dir}>" "${_EP_${dir}}" ${var} "${${var}}")
      endforeach()
    endif()
  endforeach()
endmacro()


function(_ep_command_line_to_initial_cache var args force)
  set(script_initial_cache "")
  set(regex "^([^:]+):([^=]+)=(.*)$")
  set(setArg "")
  set(forceArg "")
  if(force)
    set(forceArg "FORCE")
  endif()
  foreach(line ${args})
    if("${line}" MATCHES "^-D(.*)")
      set(line "${CMAKE_MATCH_1}")
      if(NOT "${setArg}" STREQUAL "")
        # This is required to build up lists in variables, or complete an entry
        string(APPEND setArg "${accumulator}\" CACHE ${type} \"Initial cache\" ${forceArg})")
        string(APPEND script_initial_cache "\n${setArg}")
        set(accumulator "")
        set(setArg "")
      endif()
      if("${line}" MATCHES "${regex}")
        set(name "${CMAKE_MATCH_1}")
        set(type "${CMAKE_MATCH_2}")
        set(value "${CMAKE_MATCH_3}")
        set(setArg "set(${name} \"${value}")
      else()
        message(WARNING "Line '${line}' does not match regex. Ignoring.")
      endif()
    else()
      # Assume this is a list to append to the last var
      string(APPEND accumulator ";${line}")
    endif()
  endforeach()
  # Catch the final line of the args
  if(NOT "${setArg}" STREQUAL "")
    string(APPEND setArg "${accumulator}\" CACHE ${type} \"Initial cache\" ${forceArg})")
    string(APPEND script_initial_cache "\n${setArg}")
  endif()
  set(${var} ${script_initial_cache} PARENT_SCOPE)
endfunction()


function(_ep_write_initial_cache target_name script_filename script_initial_cache)
  # Write out values into an initial cache, that will be passed to CMake with -C
  # Replace location tags.
  _ep_replace_location_tags(${target_name} script_initial_cache)
  _ep_replace_location_tags(${target_name} script_filename)
  # Replace list separators.
  get_property(sep TARGET ${target_name} PROPERTY _EP_LIST_SEPARATOR)
  if(sep AND script_initial_cache)
    string(REPLACE "${sep}" ";" script_initial_cache "${script_initial_cache}")
  endif()
  # Write out the initial cache file to the location specified.
  file(GENERATE OUTPUT "${script_filename}" CONTENT "${script_initial_cache}")
endfunction()


function(ExternalProject_Get_Property name)
  foreach(var ${ARGN})
    string(TOUPPER "${var}" VAR)
    get_property(is_set TARGET ${name} PROPERTY _EP_${VAR} SET)
    if(NOT is_set)
      message(FATAL_ERROR "External project \"${name}\" has no ${var}")
    endif()
    get_property(${var} TARGET ${name} PROPERTY _EP_${VAR})
    set(${var} "${${var}}" PARENT_SCOPE)
  endforeach()
endfunction()


function(_ep_get_configure_command_id name cfg_cmd_id_var)
  get_target_property(cmd ${name} _EP_CONFIGURE_COMMAND)

  if(cmd STREQUAL "")
    # Explicit empty string means no configure step for this project
    set(${cfg_cmd_id_var} "none" PARENT_SCOPE)
  else()
    if(NOT cmd)
      # Default is "use cmake":
      set(${cfg_cmd_id_var} "cmake" PARENT_SCOPE)
    else()
      # Otherwise we have to analyze the value:
      if(cmd MATCHES "^[^;]*/configure")
        set(${cfg_cmd_id_var} "configure" PARENT_SCOPE)
      elseif(cmd MATCHES "^[^;]*/cmake" AND NOT cmd MATCHES ";-[PE];")
        set(${cfg_cmd_id_var} "cmake" PARENT_SCOPE)
      elseif(cmd MATCHES "config")
        set(${cfg_cmd_id_var} "configure" PARENT_SCOPE)
      else()
        set(${cfg_cmd_id_var} "unknown:${cmd}" PARENT_SCOPE)
      endif()
    endif()
  endif()
endfunction()


function(_ep_get_build_command name step cmd_var)
  set(cmd "")
  set(args)
  _ep_get_configure_command_id(${name} cfg_cmd_id)
  if(cfg_cmd_id STREQUAL "cmake")
    # CMake project.  Select build command based on generator.
    get_target_property(cmake_generator ${name} _EP_CMAKE_GENERATOR)
    if("${CMAKE_GENERATOR}" MATCHES "Make" AND
       ("${cmake_generator}" MATCHES "Make" OR NOT cmake_generator))
      # The project uses the same Makefile generator.  Use recursive make.
      set(cmd "$(MAKE)")
      if(step STREQUAL "INSTALL")
        set(args install)
      endif()
      if("x${step}x" STREQUAL "xTESTx")
        set(args test)
      endif()
    else()
      # Drive the project with "cmake --build".
      get_target_property(cmake_command ${name} _EP_CMAKE_COMMAND)
      if(cmake_command)
        set(cmd "${cmake_command}")
      else()
        set(cmd "${CMAKE_COMMAND}")
      endif()
      set(args --build ".")
      get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
      if(_isMultiConfig)
        if (CMAKE_CFG_INTDIR AND
            NOT CMAKE_CFG_INTDIR STREQUAL "." AND
            NOT CMAKE_CFG_INTDIR MATCHES "\\$")
          # CMake 3.4 and below used the CMAKE_CFG_INTDIR placeholder value
          # provided by multi-configuration generators.  Some projects were
          # taking advantage of that undocumented implementation detail to
          # specify a specific configuration here.  They should use
          # BUILD_COMMAND to change the default command instead, but for
          # compatibility honor the value.
          set(config ${CMAKE_CFG_INTDIR})
          message(AUTHOR_WARNING "CMAKE_CFG_INTDIR should not be set by project code.\n"
            "To get a non-default build command, use the BUILD_COMMAND option.")
        else()
          set(config $<CONFIG>)
        endif()
        list(APPEND args --config ${config})
      endif()
      if(step STREQUAL "INSTALL")
        list(APPEND args --target install)
      endif()
      # But for "TEST" drive the project with corresponding "ctest".
      if("x${step}x" STREQUAL "xTESTx")
        string(REGEX REPLACE "^(.*/)cmake([^/]*)$" "\\1ctest\\2" cmd "${cmd}")
        set(args "")
        if(_isMultiConfig)
          list(APPEND args -C ${config})
        endif()
      endif()
    endif()
  else()
    # Non-CMake project.  Guess "make" and "make install" and "make test".
    if("${CMAKE_GENERATOR}" MATCHES "Makefiles")
      # Try to get the parallel arguments
      set(cmd "$(MAKE)")
    else()
      set(cmd "make")
    endif()
    if(step STREQUAL "INSTALL")
      set(args install)
    endif()
    if("x${step}x" STREQUAL "xTESTx")
      set(args test)
    endif()
  endif()

  # Use user-specified arguments instead of default arguments, if any.
  get_property(have_args TARGET ${name} PROPERTY _EP_${step}_ARGS SET)
  if(have_args)
    get_target_property(args ${name} _EP_${step}_ARGS)
  endif()

  if(NOT "${args}" STREQUAL "")
    # args could have empty items, so we must quote it to prevent them
    # from being silently removed
    list(APPEND cmd "${args}")
  endif()
  set(${cmd_var} "${cmd}" PARENT_SCOPE)
endfunction()

function(_ep_write_log_script name step genex_supported cmd_var)

  set(log_dir "${_EP_LOG_DIR}")
  set(tmp_dir "${_EP_TMP_DIR}")

  if(genex_supported)
    set(script_base ${tmp_dir}/${name}-${step}-$<CONFIG>)
  else()
    set(script_base ${tmp_dir}/${name}-${step})
  endif()
  set(command "${${cmd_var}}")

  set(make "")
  set(code_cygpath_make "")
  if(command MATCHES [[^\$\(MAKE\)]])
    # GNU make recognizes the string "$(MAKE)" as recursive make, so
    # ensure that it appears directly in the makefile.
    string(REGEX REPLACE [[^\$\(MAKE\)]] [[${make}]] command "${command}")
    set(make "-Dmake=$(MAKE)")

    if(WIN32 AND NOT CYGWIN)
      set(code_cygpath_make "
if(\${make} MATCHES \"^/\")
  execute_process(
    COMMAND cygpath -w \${make}
    OUTPUT_VARIABLE cygpath_make
    ERROR_VARIABLE cygpath_make
    RESULT_VARIABLE cygpath_error
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(NOT cygpath_error)
    set(make \${cygpath_make})
  endif()
endif()
")
    endif()
  endif()

  set(config "")
  if("${CMAKE_CFG_INTDIR}" MATCHES [[^\$]])
    string(REPLACE "${CMAKE_CFG_INTDIR}" [[${config}]] command "${command}")
    set(config "-Dconfig=${CMAKE_CFG_INTDIR}")
  endif()

  # Wrap multiple 'COMMAND' lines up into a second-level wrapper
  # script so all output can be sent to one log file.
  if(command MATCHES "(^|;)COMMAND;")
    set(code_execute_process "
${code_cygpath_make}
execute_process(COMMAND \${command} RESULT_VARIABLE result)
if(result)
  set(msg \"Command failed (\${result}):\\n\")
  foreach(arg IN LISTS command)
    set(msg \"\${msg} '\${arg}'\")
  endforeach()
  message(FATAL_ERROR \"\${msg}\")
endif()
")
    set(code "")
    set(cmd "")
    set(sep "")
    foreach(arg IN LISTS command)
      if("x${arg}" STREQUAL "xCOMMAND")
        if(NOT "x${cmd}" STREQUAL "x")
          string(APPEND code "set(command \"${cmd}\")${code_execute_process}")
        endif()
        set(cmd "")
        set(sep "")
      else()
        string(APPEND cmd "${sep}${arg}")
        set(sep ";")
      endif()
    endforeach()
    string(APPEND code "set(command \"${cmd}\")${code_execute_process}")
    if(genex_supported)
      file(GENERATE OUTPUT "${script_base}-impl.cmake" CONTENT "${code}")
    else()
      file(WRITE "${script_base}-impl.cmake" "${code}")
    endif()
    set(command ${CMAKE_COMMAND}
      -D "make=\${make}"
      -D "config=\${config}"
      -P ${script_base}-impl.cmake
    )
  endif()

  # Wrap the command in a script to log output to files.
  set(logbase ${log_dir}/${name}-${step})
  set(log_merged "${_EP_LOG_MERGED_STDOUTERR}")
  set(log_output_on_failure "${_EP_LOG_OUTPUT_ON_FAILURE}")
  if (log_merged)
    set(stdout_log "${logbase}.log")
    set(stderr_log "${logbase}.log")
  else()
    set(stdout_log "${logbase}-out.log")
    set(stderr_log "${logbase}-err.log")
  endif()
  set(code "
cmake_minimum_required(VERSION 3.15)
${code_cygpath_make}
set(command \"${command}\")
set(log_merged \"${log_merged}\")
set(log_output_on_failure \"${log_output_on_failure}\")
set(stdout_log \"${stdout_log}\")
set(stderr_log \"${stderr_log}\")
execute_process(
  COMMAND \${command}
  RESULT_VARIABLE result
  OUTPUT_FILE \"\${stdout_log}\"
  ERROR_FILE \"\${stderr_log}\"
  )
macro(read_up_to_max_size log_file output_var)
  file(SIZE \${log_file} determined_size)
  set(max_size 10240)
  if (determined_size GREATER max_size)
    math(EXPR seek_position \"\${determined_size} - \${max_size}\")
    file(READ \${log_file} \${output_var} OFFSET \${seek_position})
    set(\${output_var} \"...skipping to end...\\n\${\${output_var}}\")
  else()
    file(READ \${log_file} \${output_var})
  endif()
endmacro()
if(result)
  set(msg \"Command failed: \${result}\\n\")
  foreach(arg IN LISTS command)
    set(msg \"\${msg} '\${arg}'\")
  endforeach()
  if (\${log_merged})
    set(msg \"\${msg}\\nSee also\\n  \${stderr_log}\")
  else()
    set(msg \"\${msg}\\nSee also\\n  ${logbase}-*.log\")
  endif()
  if (\${log_output_on_failure})
    message(SEND_ERROR \"\${msg}\")
    if (\${log_merged})
      read_up_to_max_size(\"\${stderr_log}\" error_log_contents)
      message(STATUS \"Log output is:\\n\${error_log_contents}\")
    else()
      read_up_to_max_size(\"\${stdout_log}\" out_log_contents)
      read_up_to_max_size(\"\${stderr_log}\" err_log_contents)
      message(STATUS \"stdout output is:\\n\${out_log_contents}\")
      message(STATUS \"stderr output is:\\n\${err_log_contents}\")
    endif()
    message(FATAL_ERROR \"Stopping after outputting logs.\")
  else()
    message(FATAL_ERROR \"\${msg}\")
  endif()
else()
  if(NOT \"${CMAKE_GENERATOR}\" MATCHES \"Ninja\" AND NOT \"${_EP_QUIET}\")
    set(msg \"${name} ${step} command succeeded.  See also ${logbase}-*.log\")
    message(STATUS \"\${msg}\")
  endif()
endif()
")
  set(script_filename ${script_base}.cmake)
  if(genex_supported)
    file(GENERATE OUTPUT ${script_filename} CONTENT "${code}")
  else()
    file(WRITE ${script_filename} "${code}")
  endif()
  set(command ${CMAKE_COMMAND} ${make} ${config} -P ${script_filename})
  set(${cmd_var} "${command}" PARENT_SCOPE)
endfunction()

# This module used to use "/${CMAKE_CFG_INTDIR}" directly and produced
# makefiles with "/./" in paths for custom command dependencies. Which
# resulted in problems with parallel make -j invocations.
#
# This function was added so that the suffix (search below for ${cfgdir}) is
# only set to "/${CMAKE_CFG_INTDIR}" when ${CMAKE_CFG_INTDIR} is not going to
# be "." (multi-configuration build systems like Visual Studio and Xcode...)
#
function(_ep_get_configuration_subdir_suffix suffix_var)
  set(suffix "")
  get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(_isMultiConfig)
    set(suffix "/${CMAKE_CFG_INTDIR}")
  endif()
  set(${suffix_var} "${suffix}" PARENT_SCOPE)
endfunction()


function(_ep_get_step_stampfile name step stampfile_var)
  ExternalProject_Get_Property(${name} stamp_dir)

  _ep_get_configuration_subdir_suffix(cfgdir)
  set(stampfile "${stamp_dir}${cfgdir}/${name}-${step}")

  set(${stampfile_var} "${stampfile}" PARENT_SCOPE)
endfunction()


function(_ep_get_complete_stampfile name stampfile_var)
  set(cmf_dir ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles)
  _ep_get_configuration_subdir_suffix(cfgdir)
  set(stampfile "${cmf_dir}${cfgdir}/${name}-complete")

  set(${stampfile_var} ${stampfile} PARENT_SCOPE)
endfunction()


function(_ep_step_add_target name step no_deps)
  if(TARGET ${name}-${step})
    return()
  endif()
  get_property(cmp0114 TARGET ${name} PROPERTY _EP_CMP0114)
  _ep_get_step_stampfile(${name} ${step} stamp_file)
  cmake_policy(PUSH)
  if(cmp0114 STREQUAL "NEW")
    # To implement CMP0114 NEW behavior with Makefile generators,
    # we need CMP0113 NEW behavior.
    cmake_policy(SET CMP0113 NEW)
  endif()
  add_custom_target(${name}-${step}
    DEPENDS ${stamp_file})
  cmake_policy(POP)
  set_property(TARGET ${name}-${step} PROPERTY _EP_IS_EXTERNAL_PROJECT_STEP 1)
  set_property(TARGET ${name}-${step} PROPERTY LABELS ${name})
  set_property(TARGET ${name}-${step} PROPERTY FOLDER "ExternalProjectTargets/${name}")

  if(cmp0114 STREQUAL "NEW")
    # Add target-level dependencies for the step.
    get_property(exclude_from_main TARGET ${name} PROPERTY _EP_${step}_EXCLUDE_FROM_MAIN)
    if(NOT exclude_from_main)
      add_dependencies(${name} ${name}-${step})
    endif()
    _ep_step_add_target_dependencies(${name} ${step} ${step})
    _ep_step_add_target_dependents(${name} ${step} ${step})

    get_property(independent TARGET ${name} PROPERTY _EP_${step}_INDEPENDENT)
  else()
    if(no_deps AND "${step}" MATCHES "^(configure|build|install|test)$")
      message(AUTHOR_WARNING "Using NO_DEPENDS for \"${step}\" step  might break parallel builds")
    endif()
    set(independent ${no_deps})
  endif()

  # Depend on other external projects (target-level).
  if(NOT independent)
    get_property(deps TARGET ${name} PROPERTY _EP_DEPENDS)
    foreach(arg IN LISTS deps)
      add_dependencies(${name}-${step} ${arg})
    endforeach()
  endif()
endfunction()


function(_ep_step_add_target_dependencies name step node)
  get_property(dependees TARGET ${name} PROPERTY _EP_${node}_INTERNAL_DEPENDEES)
  list(REMOVE_DUPLICATES dependees)
  foreach(dependee IN LISTS dependees)
    get_property(exclude_from_main TARGET ${name} PROPERTY _EP_${step}_EXCLUDE_FROM_MAIN)
    get_property(dependee_dependers TARGET ${name} PROPERTY _EP_${dependee}_INTERNAL_DEPENDERS)
    if(exclude_from_main OR dependee_dependers MATCHES ";")
      # The step on which our step target depends itself has
      # dependents in multiple targes.  It needs a step target too
      # so that there is a unique place for its custom command.
      _ep_step_add_target("${name}" "${dependee}" "FALSE")
    endif()

    if(TARGET ${name}-${dependee})
      add_dependencies(${name}-${step} ${name}-${dependee})
    else()
      _ep_step_add_target_dependencies(${name} ${step} ${dependee})
    endif()
  endforeach()
endfunction()


function(_ep_step_add_target_dependents name step node)
  get_property(dependers TARGET ${name} PROPERTY _EP_${node}_INTERNAL_DEPENDERS)
  list(REMOVE_DUPLICATES dependers)
  foreach(depender IN LISTS dependers)
    if(TARGET ${name}-${depender})
      add_dependencies(${name}-${depender} ${name}-${step})
    else()
      _ep_step_add_target_dependents(${name} ${step} ${depender})
    endif()
  endforeach()
endfunction()


function(ExternalProject_Add_StepTargets name)
  get_property(cmp0114 TARGET ${name} PROPERTY _EP_CMP0114)
  set(steps ${ARGN})
  if(ARGC GREATER 1 AND "${ARGV1}" STREQUAL "NO_DEPENDS")
    set(no_deps 1)
    list(REMOVE_AT steps 0)
  else()
    set(no_deps 0)
  endif()
  if(cmp0114 STREQUAL "NEW")
    if(no_deps)
      message(FATAL_ERROR
        "The 'NO_DEPENDS' option is no longer allowed.  "
        "It has been superseded by the per-step 'INDEPENDENT' option.  "
        "See policy CMP0114."
        )
    endif()
  elseif(cmp0114 STREQUAL "")
    cmake_policy(GET_WARNING CMP0114 _cmp0114_warning)
    string(APPEND _cmp0114_warning "\n"
      "ExternalProject target '${name}' would depend on the targets for "
      "step(s) '${steps}' under policy CMP0114, but this is being left out "
      "for compatibility since the policy is not set."
      )
    if(no_deps)
      string(APPEND _cmp0114_warning
        "  Also, the NO_DEPENDS option is deprecated in favor of policy CMP0114."
        )
    endif()
    message(AUTHOR_WARNING "${_cmp0114_warning}")
  endif()
  foreach(step ${steps})
    _ep_step_add_target("${name}" "${step}" "${no_deps}")
  endforeach()
endfunction()


function(ExternalProject_Add_Step name step)
  get_property(cmp0114 TARGET ${name} PROPERTY _EP_CMP0114)
  _ep_get_complete_stampfile(${name} complete_stamp_file)
  _ep_get_step_stampfile(${name} ${step} stamp_file)

  set(keywords
    COMMAND
    COMMENT
    DEPENDEES
    DEPENDERS
    DEPENDS
    INDEPENDENT
    BYPRODUCTS
    ALWAYS
    EXCLUDE_FROM_MAIN
    WORKING_DIRECTORY
    LOG
    USES_TERMINAL
  )
  _ep_parse_arguments("${keywords}" ${name} _EP_${step}_ "${ARGN}")

  get_property(independent TARGET ${name} PROPERTY _EP_${step}_INDEPENDENT)
  if(independent STREQUAL "")
    set(independent FALSE)
    set_property(TARGET ${name} PROPERTY _EP_${step}_INDEPENDENT "${independent}")
  endif()

  get_property(exclude_from_main TARGET ${name} PROPERTY _EP_${step}_EXCLUDE_FROM_MAIN)
  if(NOT exclude_from_main)
    add_custom_command(APPEND
      OUTPUT ${complete_stamp_file}
      DEPENDS ${stamp_file}
      )
  endif()

  # Steps depending on this step.
  get_property(dependers TARGET ${name} PROPERTY _EP_${step}_DEPENDERS)
  set_property(TARGET ${name} APPEND PROPERTY _EP_${step}_INTERNAL_DEPENDERS ${dependers})
  foreach(depender IN LISTS dependers)
    set_property(TARGET ${name} APPEND PROPERTY _EP_${depender}_INTERNAL_DEPENDEES ${step})
    _ep_get_step_stampfile(${name} ${depender} depender_stamp_file)
    add_custom_command(APPEND
      OUTPUT ${depender_stamp_file}
      DEPENDS ${stamp_file}
      )
    if(cmp0114 STREQUAL "NEW" AND NOT independent)
      get_property(dep_independent TARGET ${name} PROPERTY _EP_${depender}_INDEPENDENT)
      if(dep_independent)
        message(FATAL_ERROR "ExternalProject '${name}' step '${depender}' is marked INDEPENDENT "
          "but depends on step '${step}' that is not marked INDEPENDENT.")
      endif()
    endif()
  endforeach()

  # Dependencies on files.
  get_property(depends TARGET ${name} PROPERTY _EP_${step}_DEPENDS)

  # Byproducts of the step.
  get_property(byproducts TARGET ${name} PROPERTY _EP_${step}_BYPRODUCTS)

  # Dependencies on steps.
  get_property(dependees TARGET ${name} PROPERTY _EP_${step}_DEPENDEES)
  set_property(TARGET ${name} APPEND PROPERTY _EP_${step}_INTERNAL_DEPENDEES ${dependees})
  foreach(dependee IN LISTS dependees)
    set_property(TARGET ${name} APPEND PROPERTY _EP_${dependee}_INTERNAL_DEPENDERS ${step})
    _ep_get_step_stampfile(${name} ${dependee} dependee_stamp_file)
    list(APPEND depends ${dependee_stamp_file})
    if(cmp0114 STREQUAL "NEW" AND independent)
      get_property(dep_independent TARGET ${name} PROPERTY _EP_${dependee}_INDEPENDENT)
      if(NOT dep_independent)
        message(FATAL_ERROR "ExternalProject '${name}' step '${step}' is marked INDEPENDENT "
          "but depends on step '${dependee}' that is not marked INDEPENDENT.")
      endif()
    endif()
  endforeach()

  # The command to run.
  get_property(command TARGET ${name} PROPERTY _EP_${step}_COMMAND)
  if(command)
    set(comment "Performing ${step} step for '${name}'")
  else()
    set(comment "No ${step} step for '${name}'")
  endif()
  get_property(work_dir TARGET ${name} PROPERTY _EP_${step}_WORKING_DIRECTORY)

  # Replace list separators.
  get_property(sep TARGET ${name} PROPERTY _EP_LIST_SEPARATOR)
  if(sep AND command)
    string(REPLACE "${sep}" "\\;" command "${command}")
  endif()

  # Replace location tags.
  _ep_replace_location_tags(${name} comment command work_dir byproducts)

  # Custom comment?
  get_property(comment_set TARGET ${name} PROPERTY _EP_${step}_COMMENT SET)
  if(comment_set)
    get_property(comment TARGET ${name} PROPERTY _EP_${step}_COMMENT)
  endif()

  # Uses terminal?
  get_property(uses_terminal TARGET ${name} PROPERTY _EP_${step}_USES_TERMINAL)
  if(uses_terminal)
    set(uses_terminal USES_TERMINAL)
  else()
    set(uses_terminal "")
  endif()

  # Run every time?
  get_property(always TARGET ${name} PROPERTY _EP_${step}_ALWAYS)
  if(always)
    set_property(SOURCE ${stamp_file} PROPERTY SYMBOLIC 1)
    set(touch)
    # Remove any existing stamp in case the option changed in an existing tree.
    get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(_isMultiConfig)
      foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
        string(REPLACE "/${CMAKE_CFG_INTDIR}" "/${cfg}" stamp_file_config "${stamp_file}")
        file(REMOVE ${stamp_file_config})
      endforeach()
    else()
      file(REMOVE ${stamp_file})
    endif()
  else()
    set(touch ${CMAKE_COMMAND} -E touch ${stamp_file})
  endif()

  # Wrap with log script?
  get_property(log TARGET ${name} PROPERTY _EP_${step}_LOG)
  if(command AND log)
    set(genex_supported TRUE)
    _ep_write_log_script(${name} ${step} ${genex_supported} command)
  endif()

  if("${command}" STREQUAL "")
    # Some generators (i.e. Xcode) will not generate a file level target
    # if no command is set, and therefore the dependencies on this
    # target will be broken.
    # The empty command is replaced by an echo command here in order to
    # avoid this issue.
    set(command ${CMAKE_COMMAND} -E echo_append)
  endif()

  set(__cmdQuoted)
  foreach(__item IN LISTS command)
    string(APPEND __cmdQuoted " [==[${__item}]==]")
  endforeach()
  cmake_language(EVAL CODE "
    add_custom_command(
      OUTPUT \${stamp_file}
      BYPRODUCTS \${byproducts}
      COMMENT \${comment}
      COMMAND ${__cmdQuoted}
      COMMAND \${touch}
      DEPENDS \${depends}
      WORKING_DIRECTORY \${work_dir}
      VERBATIM
      ${uses_terminal}
    )"
  )
  set_property(TARGET ${name} APPEND PROPERTY _EP_STEPS ${step})

  # Add custom "step target"?
  get_property(step_targets TARGET ${name} PROPERTY _EP_STEP_TARGETS)
  if(NOT step_targets)
    get_property(step_targets DIRECTORY PROPERTY EP_STEP_TARGETS)
  endif()
  foreach(st ${step_targets})
    if("${st}" STREQUAL "${step}")
      _ep_step_add_target("${name}" "${step}" "FALSE")
      break()
    endif()
  endforeach()

  get_property(independent_step_targets TARGET ${name} PROPERTY _EP_INDEPENDENT_STEP_TARGETS)
  if(NOT independent_step_targets)
    get_property(independent_step_targets DIRECTORY PROPERTY EP_INDEPENDENT_STEP_TARGETS)
  endif()
  if(cmp0114 STREQUAL "NEW")
    if(independent_step_targets)
      message(FATAL_ERROR
        "ExternalProject '${name}' option 'INDEPENDENT_STEP_TARGETS' is set to\n"
        "  ${independent_step_targets}\n"
        "but the option is no longer allowed.  "
        "It has been superseded by the per-step 'INDEPENDENT' option.  "
        "See policy CMP0114."
        )
    endif()
  else()
    if(independent_step_targets AND cmp0114 STREQUAL "")
      get_property(warned TARGET ${name} PROPERTY _EP_CMP0114_WARNED_INDEPENDENT_STEP_TARGETS)
      if(NOT warned)
        set_property(TARGET ${name} PROPERTY _EP_CMP0114_WARNED_INDEPENDENT_STEP_TARGETS 1)
        cmake_policy(GET_WARNING CMP0114 _cmp0114_warning)
        string(APPEND _cmp0114_warning "\n"
          "ExternalProject '${name}' option INDEPENDENT_STEP_TARGETS is set to\n"
          "  ${independent_step_targets}\n"
          "but the option is deprecated in favor of policy CMP0114."
          )
        message(AUTHOR_WARNING "${_cmp0114_warning}")
      endif()
    endif()
    foreach(st ${independent_step_targets})
      if("${st}" STREQUAL "${step}")
        _ep_step_add_target("${name}" "${step}" "TRUE")
        break()
      endif()
    endforeach()
  endif()
endfunction()


function(ExternalProject_Add_StepDependencies name step)
  set(dependencies ${ARGN})

  # Sanity checks on "name" and "step".
  if(NOT TARGET ${name})
    message(FATAL_ERROR "Cannot find target \"${name}\". Perhaps it has not yet been created using ExternalProject_Add.")
  endif()

  get_property(type TARGET ${name} PROPERTY TYPE)
  if(NOT type STREQUAL "UTILITY")
    message(FATAL_ERROR "Target \"${name}\" was not generated by ExternalProject_Add.")
  endif()

  get_property(is_ep TARGET ${name} PROPERTY _EP_IS_EXTERNAL_PROJECT)
  if(NOT is_ep)
    message(FATAL_ERROR "Target \"${name}\" was not generated by ExternalProject_Add.")
  endif()

  get_property(steps TARGET ${name} PROPERTY _EP_STEPS)
  list(FIND steps ${step} is_step)
  if(is_step LESS 0)
    message(FATAL_ERROR "External project \"${name}\" does not have a step \"${step}\".")
  endif()

  if(TARGET ${name}-${step})
    get_property(type TARGET ${name}-${step} PROPERTY TYPE)
    if(NOT type STREQUAL "UTILITY")
      message(FATAL_ERROR "Target \"${name}-${step}\" was not generated by ExternalProject_Add_StepTargets.")
    endif()
    get_property(is_ep_step TARGET ${name}-${step} PROPERTY _EP_IS_EXTERNAL_PROJECT_STEP)
    if(NOT is_ep_step)
      message(FATAL_ERROR "Target \"${name}-${step}\" was not generated by ExternalProject_Add_StepTargets.")
    endif()
  endif()

  # Always add file-level dependency, but add target-level dependency
  # only if the target exists for that step.
  _ep_get_step_stampfile(${name} ${step} stamp_file)
  foreach(dep ${dependencies})
    add_custom_command(APPEND
      OUTPUT ${stamp_file}
      DEPENDS ${dep})
    if(TARGET ${name}-${step})
      foreach(dep ${dependencies})
        add_dependencies(${name}-${step} ${dep})
      endforeach()
    endif()
  endforeach()

endfunction()

function(_ep_is_dir_empty dir empty_var)
  file(GLOB gr "${dir}/*")
  if("${gr}" STREQUAL "")
    set(${empty_var} 1 PARENT_SCOPE)
  else()
    set(${empty_var} 0 PARENT_SCOPE)
  endif()
endfunction()

function(_ep_get_git_submodules_recurse git_submodules_recurse)

  if(NOT DEFINED _EP_GIT_SUBMODULES_RECURSE OR _EP_GIT_SUBMODULES_RECURSE)
    # The git submodule update '--recursive' flag requires git >= v1.6.5
    if(recurseFlag AND GIT_VERSION_STRING VERSION_LESS 1.6.5)
      message(FATAL_ERROR
        "git version 1.6.5 or later required for --recursive flag with "
        "'git submodule ...': GIT_VERSION_STRING='${GIT_VERSION_STRING}'")
    endif()
    set(${git_submodules_recurse} "--recursive" PARENT_SCOPE)
  else()
    set(${git_submodules_recurse} "" PARENT_SCOPE)
  endif()

endfunction()

function(_ep_write_command_script
         script_filename
         commands
         work_dir
         genex_supported
         quiet
         have_commands_var)

  set(sep "${_EP_LIST_SEPARATOR}")
  if(sep AND commands)
    string(REPLACE "${sep}" "\\;" commands "${commands}")
  endif()
  _ep_replace_location_tags_from_vars(commands)

  file(READ
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/customcommand.cmake.in
    exec_command_template
  )
  set(script_content)
  set(this_command)
  foreach(token IN LISTS commands)
    if(token STREQUAL "COMMAND")
      if("${this_command}" STREQUAL "")
        # Silently skip empty commands
        continue()
      endif()
      string(CONFIGURE "${exec_command_template}" content @ONLY)
      string(APPEND script_content "${content}")
      set(this_command)
    else()
      # Ensure we quote every token so we preserve empty items, quotes, etc
      string(APPEND this_command " [==[${token}]==]")
    endif()
  endforeach()

  if(NOT "${this_command}" STREQUAL "")
    string(CONFIGURE "${exec_command_template}" content @ONLY)
    string(APPEND script_content "${content}")
  endif()

  if(script_content STREQUAL "")
    set(${have_commands_var} FALSE PARENT_SCOPE)
  else()
    set(${have_commands_var} TRUE PARENT_SCOPE)
    file(READ
      ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/customcommand_preamble.cmake.in
      exec_command_preamble
    )
    string(CONFIGURE "${exec_command_preamble}" exec_command_preamble @ONLY)
    string(PREPEND script_content "${exec_command_preamble}")
  endif()

  if(genex_supported)
    # Only written at generation phase. This will only change the file's
    # timestamp if the contents change.
    file(GENERATE OUTPUT "${script_filename}" CONTENT "${script_content}")
  else()
    # Update the file immediately, needed if script has to be invoked in the
    # configure phase (e.g. via FetchContent). We need to be careful to avoid
    # updating the timestamp if the file contents don't change. The file(WRITE)
    # command always updates the file, so avoid it if we don't need to call it.
    set(doWrite TRUE)
    if(EXISTS "${script_filename}")
      file(READ "${script_filename}" existing_content)
      if(existing_content STREQUAL script_content)
        set(doWrite FALSE)
      endif()
    endif()
    if(doWrite)
      file(WRITE "${script_filename}" "${script_content}")
    endif()
  endif()

endfunction()

function(_ep_add_preconfigure_command name step)

  string(TOUPPER "${step}" STEP)
  set(uses_terminal "${_EP_USES_TERMINAL_${STEP}}")
  if(uses_terminal)
    set(uses_terminal TRUE)
  else()
    set(uses_terminal FALSE)
  endif()

  # Pre-configure steps are expected to set their own work_dir
  ExternalProject_Add_Step(${name} ${step}
    INDEPENDENT        TRUE
    COMMENT           "${_EPcomment_${STEP}}"
    COMMAND            ${_EPcommand_${STEP}}
    ALWAYS             ${_EPalways_${STEP}}
    EXCLUDE_FROM_MAIN  ${_EPexcludefrommain_${STEP}}
    DEPENDS            ${_EPdepends_${STEP}}
    DEPENDEES          ${_EPdependees_${STEP}}
    USES_TERMINAL      ${uses_terminal}
  )
endfunction()

# This function is an implementation detail of ExternalProject_Add() and
# _ep_do_preconfigure_steps_now().
#
# The function expects keyword arguments to have already been parsed into
# variables of the form _EP_<keyword>. It will populate the variable
# _EP_DOWNLOADED_FILE in the calling scope only if the download method is
# URL-based and extraction has been turned off.
#
# Variables will also be set in the calling scope to enable subsequently
# calling _ep_add_preconfigure_command() for the download step.
#
function(_ep_prepare_download name genex_supported)

  set(stamp_dir    "${_EP_STAMP_DIR}")
  set(tmp_dir      "${_EP_TMP_DIR}")
  set(source_dir   "${_EP_SOURCE_DIR}")
  set(download_dir "${_EP_DOWNLOAD_DIR}")
  set(quiet        "${_EP_QUIET}")

  set(comment)

  # We handle the log setting directly here rather than deferring it to
  # be handled by ExternalProject_Add_Step()
  set(log "${_EP_LOG_DOWNLOAD}")
  if(log)
    set(script_filename ${tmp_dir}/${name}-download-impl.cmake)
    set(log TRUE)
    set(quiet FALSE)  # Already quiet as a result of log being enabled
  else()
    set(script_filename ${tmp_dir}/${name}-download.cmake)
    set(log FALSE)
  endif()

  set(repo_info_file  ${tmp_dir}/${name}-download-repoinfo.txt)
  set(last_run_file ${stamp_dir}/${name}-download-lastrun.txt)
  set(script_does_something TRUE)

  # We use configure_file() to write the repo_info_file below so that the
  # file's timestamp is not updated if we don't change the contents of an
  # existing file.

  if(DEFINED _EP_DOWNLOAD_COMMAND)
    set(work_dir ${download_dir})
    set(repo_info_content
"method=custom
command=${_EP_DOWNLOAD_COMMAND}
source_dir=${source_dir}
work_dir=${work_dir}
")
    configure_file(
      ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/RepositoryInfo.txt.in
      ${repo_info_file}
      @ONLY
    )

    _ep_write_command_script(
      "${script_filename}"
      "${_EP_DOWNLOAD_COMMAND}"
      "${work_dir}"
      "${genex_supported}"
      "${quiet}"
      script_does_something
    )
    set(comment "Performing download step (custom command) for '${name}'")

  elseif(DEFINED _EP_CVS_REPOSITORY)
    find_package(CVS QUIET)
    if(NOT CVS_EXECUTABLE)
      message(FATAL_ERROR "error: could not find cvs for checkout of ${name}")
    endif()

    if("${_EP_CVS_MODULE}" STREQUAL "")
      message(FATAL_ERROR "error: no CVS_MODULE")
    endif()

    set(repo_info_content
"method=cvs
repository=${_EP_CVS_REPOSITORY}
module=${_EP_CVS_MODULE}
tag=${_EP_CVS_TAG}
source_dir=${source_dir}
")
    configure_file(
      ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/RepositoryInfo.txt.in
      ${repo_info_file}
      @ONLY
    )

    get_filename_component(src_name "${source_dir}" NAME)
    get_filename_component(work_dir "${source_dir}" PATH)

    set(cmd "${CVS_EXECUTABLE}" -d "${_EP_CVS_REPOSITORY}" -q
      co ${_EP_CVS_TAG} -d "${src_name}" "${_EP_CVS_MODULE}"
    )
    _ep_write_command_script(
      "${script_filename}"
      "${cmd}"
      "${work_dir}"
      "${genex_supported}"
      "${quiet}"
      script_does_something
    )
    set(comment "Performing download step (CVS checkout) for '${name}'")

  elseif(DEFINED _EP_SVN_REPOSITORY)
    find_package(Subversion QUIET)
    if(NOT Subversion_SVN_EXECUTABLE)
      message(FATAL_ERROR "error: could not find svn for checkout of ${name}")
    endif()

    set(svn_repository "${_EP_SVN_REPOSITORY}")
    set(svn_revision   "${_EP_SVN_REVISION}")
    set(svn_username   "${_EP_SVN_USERNAME}")
    set(svn_password   "${_EP_SVN_PASSWORD}")
    set(svn_trust_cert "${_EP_SVN_TRUST_CERT}")

    set(svn_options --non-interactive)
    if(DEFINED _EP_SVN_USERNAME)
      list(APPEND svn_options "--username=${svn_username}")
    endif()
    if(DEFINED _EP_SVN_PASSWORD)
      list(APPEND svn_options "--password=${svn_password}")
    endif()
    if(svn_trust_cert)
      list(APPEND svn_options --trust-server-cert)
    endif()

    set(repo_info_content
"method=svn
repository=${svn_repository}
user=${svn_username}
password=${svn_password}
revision=${svn_revision}
source_dir=${source_dir}
")
    configure_file(
      ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/RepositoryInfo.txt.in
      ${repo_info_file}
      @ONLY
    )

    get_filename_component(src_name "${source_dir}" NAME)
    get_filename_component(work_dir "${source_dir}" PATH)

    set(cmd "${Subversion_SVN_EXECUTABLE}" co "${svn_repository}"
      ${svn_revision} ${svn_options} "${src_name}"
    )
    _ep_write_command_script(
      "${script_filename}"
      "${cmd}"
      "${work_dir}"
      "${genex_supported}"
      "${quiet}"
      script_does_something
    )
    set(comment "Performing download step (SVN checkout) for '${name}'")

  elseif(DEFINED _EP_GIT_REPOSITORY)
    # FetchContent gives us these directly, so don't try to recompute them
    if(NOT GIT_EXECUTABLE OR NOT GIT_VERSION_STRING)
      unset(CMAKE_MODULE_PATH) # Use CMake builtin find module
      find_package(Git QUIET)
      if(NOT GIT_EXECUTABLE)
        message(FATAL_ERROR "error: could not find git for clone of ${name}")
      endif()
    endif()

    set(git_tag "${_EP_GIT_TAG}")
    if(NOT git_tag)
      set(git_tag "master")
    endif()

    set(git_remote_name "${_EP_GIT_REMOTE_NAME}")
    if(NOT git_remote_name)
      set(git_remote_name "origin")
    endif()

    set(git_init_submodules TRUE)
    if(DEFINED _EP_GIT_SUBMODULES)
      set(git_submodules "${_EP_GIT_SUBMODULES}")
      if(git_submodules STREQUAL "" AND _EP_CMP0097 STREQUAL "NEW")
        set(git_init_submodules FALSE)
      endif()
    endif()
    _ep_get_git_submodules_recurse(git_submodules_recurse)

    set(tls_verify "${_EP_TLS_VERIFY}")
    if("x${tls_verify}" STREQUAL "x" AND DEFINED CMAKE_TLS_VERIFY)
      set(tls_verify "${CMAKE_TLS_VERIFY}")
    endif()
    set(git_shallow  "${_EP_GIT_SHALLOW}")
    set(git_progress "${_EP_GIT_PROGRESS}")
    set(git_config   "${_EP_GIT_CONFIG}")

    # Make checkouts quiet when checking out a git hash (this avoids the
    # very noisy detached head message)
    list(PREPEND git_config advice.detachedHead=false)

    # For the git clone operation, only the repository and remote should be
    # recorded in a configured repository info file. If the repo or remote
    # name changes, the clone script should be run again. But if only the tag
    # changes, avoid running the clone script again. Let the 'always' running
    # update step checkout the new tag.
    set(repo_info_content
"method=git
repository=${_EP_GIT_REPOSITORY}
remote=${git_remote_name}
source_dir=${source_dir}
")
    configure_file(
      ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/RepositoryInfo.txt.in
      ${repo_info_file}
      @ONLY
    )

    get_filename_component(src_name "${source_dir}" NAME)
    get_filename_component(work_dir "${source_dir}" PATH)

    # Since git clone doesn't succeed if the non-empty source_dir exists,
    # the script will delete the source directory and then call git clone.
    _ep_write_gitclone_script(
      "${script_filename}"
      "${source_dir}"
      "${GIT_EXECUTABLE}"
      "${_EP_GIT_REPOSITORY}"
      "${git_tag}"
      "${git_remote_name}"
      "${git_init_submodules}"
      "${git_submodules_recurse}"
      "${git_submodules}"
      "${git_shallow}"
      "${git_progress}"
      "${git_config}"
      "${src_name}"
      "${work_dir}"
      "${repo_info_file}"
      "${last_run_file}"
      "${tls_verify}"
      "${quiet}"
    )
    set(comment "Performing download step (git clone) for '${name}'")

  elseif(DEFINED _EP_HG_REPOSITORY)
    find_package(Hg QUIET)
    if(NOT HG_EXECUTABLE)
      message(FATAL_ERROR "error: could not find hg for clone of ${name}")
    endif()

    set(hg_tag "${_EP_HG_TAG}")
    if(NOT hg_tag)
      set(hg_tag "tip")
    endif()

    # For the hg clone operation, only the repository should be recorded in a
    # configured repository info file. If the repo changes, the clone script
    # should be run again. But if only the tag changes, avoid running the
    # clone script again. Let the 'always' running update step checkout the
    # new tag.
    set(repo_info_content
"method=hg
repository=${_EP_HG_REPOSITORY}
source_dir=${source_dir}
")
    configure_file(
      ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/RepositoryInfo.txt.in
      ${repo_info_file}
      @ONLY
    )

    get_filename_component(src_name "${source_dir}" NAME)
    get_filename_component(work_dir "${source_dir}" PATH)

    # Since hg clone doesn't succeed if the non-empty source_dir exists,
    # the script will delete the source directory and then call hg clone.
    _ep_write_hgclone_script(
      "${script_filename}"
      "${source_dir}"
      "${HG_EXECUTABLE}"
      "${_EP_HG_REPOSITORY}"
      "${hg_tag}"
      "${src_name}"
      "${work_dir}"
      "${repo_info_file}"
      "${last_run_file}"
      "${quiet}"
    )
    set(comment "Performing download step (hg clone) for '${name}'")

  elseif(DEFINED _EP_URL)
    set(url "${_EP_URL}")
    # TODO: Perhaps file:// should be copied to download dir before extraction.
    string(REGEX REPLACE "file://" "" url "${url}")

    set(hash "${_EP_URL_HASH}")
    _ep_get_hash_regex(_ep_hash_regex)
    if(hash AND NOT "${hash}" MATCHES "${_ep_hash_regex}")
      _ep_get_hash_algos(_ep_hash_algos)
      list(JOIN _ep_hash_algos "|" _ep_hash_algos)
      message(FATAL_ERROR "URL_HASH is set to\n  ${hash}\n"
        "but must be ALGO=value where ALGO is\n  ${_ep_hash_algos}\n"
        "and value is a hex string.")
    endif()
    set(md5 "${_EP_URL_MD5}")
    if(md5 AND NOT "MD5=${md5}" MATCHES "${_ep_hash_regex}")
      message(FATAL_ERROR "URL_MD5 is set to\n  ${md5}\nbut must be a hex string.")
    endif()
    if(md5 AND NOT hash)
      set(hash "MD5=${md5}")
    endif()

    set(repo_info_content
"method=url
url=${url}
hash=${hash}
source_dir=${source_dir}
")
    configure_file(
      ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/RepositoryInfo.txt.in
      ${repo_info_file}
      @ONLY
    )

    set(fname "${_EP_DOWNLOAD_NAME}")

    list(LENGTH url url_list_length)
    if(NOT "${url_list_length}" STREQUAL "1")
      foreach(entry ${url})
        if(NOT "${entry}" MATCHES "^[a-z]+://")
          message(FATAL_ERROR "At least one entry of URL is a path (invalid in a list)")
        endif()
      endforeach()
      if("x${fname}" STREQUAL "x")
        list(GET url 0 fname)
      endif()
    endif()

    if(IS_DIRECTORY "${url}")
      get_filename_component(from_dir "${url}" ABSOLUTE)
      _ep_write_copydir_script(
        ${script_filename}
        ${from_dir}
        ${source_dir}
      )
      set(steps "DIR copy")
    else()
      set(no_extract "${_EP_DOWNLOAD_NO_EXTRACT}")
      if(no_extract)
        set(extract_script)
      else()
        set(extract_script "${tmp_dir}/extract-${name}.cmake")
      endif()

      if("${url}" MATCHES "^[a-z]+://")
        # TODO: Should download and extraction be different steps?
        if("x${fname}" STREQUAL "x")
          set(fname "${url}")
        endif()
        if("${fname}" MATCHES [[([^/\?#]+(\.|=)(7z|tar|tar\.bz2|tar\.gz|tar\.xz|tbz2|tgz|txz|zip))([/?#].*)?$]])
          set(fname "${CMAKE_MATCH_1}")
        elseif(no_extract)
          get_filename_component(fname "${fname}" NAME)
        else()
          # Fall back to a default file name.  The actual file name does not
          # matter because it is used only internally and our extraction tool
          # inspects the file content directly.  If it turns out the wrong URL
          # was given, that will be revealed when the download is attempted
          # (during the build unless we are being invoked by FetchContent)
          # which is an easier place for users to diagnose than an error here.
          set(fname "archive.tar")
        endif()
        string(REPLACE ";" "-" fname "${fname}")
        set(file ${download_dir}/${fname})
        _ep_write_downloadfile_script(
          "${script_filename}"
          "${url}"
          "${file}"
          "${_EP_TIMEOUT}"
          "${_EP_INACTIVITY_TIMEOUT}"
          "${_EP_DOWNLOAD_NO_PROGRESS}"
          "${hash}"
          "${_EP_TLS_VERIFY}"
          "${_EP_TLS_CAINFO}"
          "${_EP_HTTP_USERNAME}:${_EP_HTTP_PASSWORD}"
          "${_EP_HTTP_HEADER}"
          "${_EP_NETRC}"
          "${_EP_NETRC_FILE}"
          "${extract_script}"
          "${quiet}"
        )
        if(no_extract)
          set(steps "download and verify")
        else()
          set(steps "download, verify and extract")
        endif()
      else()
        set(file "${url}")
        _ep_write_verifyfile_script(
          "${script_filename}"
          "${file}"
          "${hash}"
          "${extract_script}"
          "${quiet}"
        )
        if(no_extract)
          set(steps "verify")
        else()
          set(steps "verify and extract")
        endif()
      endif()

      if(no_extract)
        set(_EP_DOWNLOADED_FILE ${file} PARENT_SCOPE)
      else()
        # This will be pulled in by the download/verify script written above
        _ep_write_extractfile_script(
          "${extract_script}"
          "${name}"
          "${file}"
          "${source_dir}"
          "${quiet}"
        )
      endif()
    endif()
    set(comment "Performing download step (${steps}) for '${name}'")

  else()
    _ep_is_dir_empty("${source_dir}" empty)
    if(${empty})
      message(FATAL_ERROR
        "No download info given for '${name}' and its source directory:\n"
        " ${source_dir}\n"
        "is not an existing non-empty directory.  Please specify one of:\n"
        " * SOURCE_DIR with an existing non-empty directory\n"
        " * DOWNLOAD_COMMAND\n"
        " * URL\n"
        " * GIT_REPOSITORY\n"
        " * SVN_REPOSITORY\n"
        " * HG_REPOSITORY\n"
        " * CVS_REPOSITORY and CVS_MODULE"
        )
    endif()
    set(repo_info_content
"method=source_dir
source_dir=${source_dir}
")
    configure_file(
      ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/RepositoryInfo.txt.in
      ${repo_info_file}
      @ONLY
    )

    set(comment "Skipping download step (SOURCE_DIR given) for '${name}'")
    set(script_does_something FALSE)
  endif()

  # Provide variables that can be used later to create a custom command or
  # invoke the step directly
  if(script_does_something)
    set(cmd ${CMAKE_COMMAND} -P ${script_filename})
    if(log)
      _ep_write_log_script(${name} download "${genex_supported}" cmd)
    endif()
    set(depends ${repo_info_file})
  else()
    set(cmd)
    set(depends)
    string(REPLACE "Performing" "Skipping" comment "${comment}")
  endif()

  set(_EPcomment_DOWNLOAD         "${comment}" PARENT_SCOPE)
  set(_EPcommand_DOWNLOAD         "${cmd}"     PARENT_SCOPE)
  set(_EPalways_DOWNLOAD          FALSE        PARENT_SCOPE)
  set(_EPexcludefrommain_DOWNLOAD FALSE        PARENT_SCOPE)
  set(_EPdepends_DOWNLOAD         "${depends}" PARENT_SCOPE)
  set(_EPdependees_DOWNLOAD       mkdir        PARENT_SCOPE)

endfunction()

function(_ep_get_update_disconnected var)
  if(DEFINED _EP_UPDATE_DISCONNECTED)
    set(update_disconnected "${_EP_UPDATE_DISCONNECTED}")
  else()
    get_property(update_disconnected DIRECTORY PROPERTY EP_UPDATE_DISCONNECTED)
  endif()
  set(${var} "${update_disconnected}" PARENT_SCOPE)
endfunction()

# This function is an implementation detail of ExternalProject_Add() and
# _ep_do_preconfigure_steps_now().
#
# The function expects keyword arguments to have already been parsed into
# variables of the form _EP_<keyword>.
#
# Variables will also be set in the calling scope to enable subsequently
# calling _ep_add_preconfigure_command() for the updated step.
#
function(_ep_prepare_update name genex_supported)

  set(tmp_dir    "${_EP_TMP_DIR}")
  set(source_dir "${_EP_SOURCE_DIR}")
  set(quiet      "${_EP_QUIET}")

  set(comment)

  _ep_get_update_disconnected(update_disconnected)

  # We handle the log setting directly here rather than deferring it to
  # be handled by ExternalProject_Add_Step()
  set(log "${_EP_LOG_UPDATE}")
  if(log)
    set(script_filename ${tmp_dir}/${name}-update-impl.cmake)
    set(log TRUE)
    set(quiet FALSE)  # Already quiet as a result of log being enabled
  else()
    set(script_filename ${tmp_dir}/${name}-update.cmake)
    set(log FALSE)
  endif()

  if(DEFINED _EP_UPDATE_COMMAND)
    set(work_dir ${source_dir})
    _ep_write_command_script(
      "${script_filename}"
      "${_EP_UPDATE_COMMAND}"
      "${work_dir}"
      "${genex_supported}"
      "${quiet}"
      script_does_something
    )
    set(comment "Performing update step (custom command) for '${name}'")

  elseif(DEFINED _EP_CVS_REPOSITORY)
    if(NOT CVS_EXECUTABLE)
      message(FATAL_ERROR "error: could not find cvs for update of ${name}")
    endif()

    set(work_dir ${source_dir})
    set(cmd "${CVS_EXECUTABLE}" -d "${_EP_CVS_REPOSITORY}" -q
      up -dP ${_EP_CVS_TAG}
    )
    _ep_write_command_script(
      "${script_filename}"
      "${cmd}"
      "${work_dir}"
      "${genex_supported}"
      "${quiet}"
      script_does_something
    )
    set(comment "Performing update step (CVS update) for '${name}'")

  elseif(DEFINED _EP_SVN_REPOSITORY)
    if(NOT Subversion_SVN_EXECUTABLE)
      message(FATAL_ERROR "error: could not find svn for update of ${name}")
    endif()

    set(svn_revision   "${_EP_SVN_REVISION}")
    set(svn_username   "${_EP_SVN_USERNAME}")
    set(svn_password   "${_EP_SVN_PASSWORD}")
    set(svn_trust_cert "${_EP_SVN_TRUST_CERT}")

    set(svn_options --non-interactive)
    if(DEFINED _EP_SVN_USERNAME)
      list(APPEND svn_options "--username=${svn_username}")
    endif()
    if(DEFINED _EP_SVN_PASSWORD)
      list(APPEND svn_options "--password=${svn_password}")
    endif()
    if(svn_trust_cert)
      list(APPEND svn_options --trust-server-cert)
    endif()

    set(work_dir ${source_dir})
    set(cmd "${Subversion_SVN_EXECUTABLE}" up ${svn_revision} ${svn_options})

    _ep_write_command_script(
      "${script_filename}"
      "${cmd}"
      "${work_dir}"
      "${genex_supported}"
      "${quiet}"
      script_does_something
    )
    set(comment "Performing update step (SVN update) for '${name}'")

  elseif(DEFINED _EP_GIT_REPOSITORY)
    # FetchContent gives us these directly, so don't try to recompute them
    if(NOT GIT_EXECUTABLE OR NOT GIT_VERSION_STRING)
      unset(CMAKE_MODULE_PATH) # Use CMake builtin find module
      find_package(Git QUIET)
      if(NOT GIT_EXECUTABLE)
        message(FATAL_ERROR "error: could not find git for fetch of ${name}")
      endif()
    endif()

    set(git_tag "${_EP_GIT_TAG}")
    if(NOT git_tag)
      set(git_tag "master")
    endif()

    set(git_remote_name "${_EP_GIT_REMOTE_NAME}")
    if(NOT git_remote_name)
      set(git_remote_name "origin")
    endif()

    set(git_init_submodules TRUE)
    if(DEFINED _EP_GIT_SUBMODULES)
      set(git_submodules "${_EP_GIT_SUBMODULES}")
      if(git_submodules  STREQUAL "" AND _EP_CMP0097 STREQUAL "NEW")
        set(git_init_submodules FALSE)
      endif()
    endif()
    _ep_get_git_submodules_recurse(git_submodules_recurse)

    set(git_update_strategy "${_EP_GIT_REMOTE_UPDATE_STRATEGY}")
    if(NOT git_update_strategy)
      set(git_update_strategy "${CMAKE_EP_GIT_REMOTE_UPDATE_STRATEGY}")
    endif()
    if(NOT git_update_strategy)
      set(git_update_strategy REBASE)
    endif()
    set(strategies CHECKOUT REBASE REBASE_CHECKOUT)
    if(NOT git_update_strategy IN_LIST strategies)
      message(FATAL_ERROR "'${git_update_strategy}' is not one of the supported strategies: ${strategies}")
    endif()

    set(work_dir ${source_dir})
    _ep_write_gitupdate_script(
      "${script_filename}"
      "${GIT_EXECUTABLE}"
      "${git_tag}"
      "${git_remote_name}"
      "${git_init_submodules}"
      "${git_submodules_recurse}"
      "${git_submodules}"
      "${_EP_GIT_REPOSITORY}"
      "${work_dir}"
      "${git_update_strategy}"
      "${quiet}"
    )
    set(script_does_something TRUE)
    set(comment "Performing update step (git update) for '${name}'")

  elseif(DEFINED _EP_HG_REPOSITORY)
    if(NOT HG_EXECUTABLE)
      message(FATAL_ERROR "error: could not find hg for pull of ${name}")
    endif()

    if("${HG_VERSION_STRING}" STREQUAL "2.1")
      message(WARNING "Mercurial 2.1 does not distinguish an empty pull from a failed pull:
 http://mercurial.selenic.com/wiki/UpgradeNotes#A2.1.1:_revert_pull_return_code_change.2C_compile_issue_on_OS_X
 http://thread.gmane.org/gmane.comp.version-control.mercurial.devel/47656
Update to Mercurial >= 2.1.1.
")
    endif()

    set(hg_tag "${_EP_HG_TAG}")
    if(NOT hg_tag)
      set(hg_tag "tip")
    endif()

    set(work_dir ${source_dir})
    _ep_write_hgupdate_script(
      "${script_filename}"
      "${HG_EXECUTABLE}"
      "${hg_tag}"
      "${work_dir}"
      "${quiet}"
    )
    set(script_does_something TRUE)
    set(comment "Performing update step (hg pull) for '${name}'")

  else()
    set(script_does_something FALSE)
  endif()

  # Provide variables that can be used later to create a custom command or
  # invoke the step directly
  if(script_does_something)
    set(always TRUE)
    set(cmd ${CMAKE_COMMAND} -P ${script_filename})
    if(log)
      _ep_write_log_script(${name} update "${genex_supported}" cmd)
    endif()
  else()
    set(always FALSE)
    set(cmd)
  endif()

  set(_EPcomment_UPDATE         "${comment}"  PARENT_SCOPE)
  set(_EPcommand_UPDATE         "${cmd}"      PARENT_SCOPE)
  set(_EPalways_UPDATE          "${always}"   PARENT_SCOPE)
  set(_EPexcludefrommain_UPDATE "${update_disconnected}" PARENT_SCOPE)
  set(_EPdepends_UPDATE         ""            PARENT_SCOPE)
  set(_EPdependees_UPDATE       download      PARENT_SCOPE)

endfunction()

# This function is an implementation detail of ExternalProject_Add() and
# _ep_do_preconfigure_steps_now().
#
# The function expects keyword arguments to have already been parsed into
# variables of the form _EP_<keyword>.
#
# Variables will also be set in the calling scope to enable subsequently
# calling _ep_add_preconfigure_command() for the patch step.
#
function(_ep_prepare_patch name genex_supported)

  set(tmp_dir    "${_EP_TMP_DIR}")
  set(source_dir "${_EP_SOURCE_DIR}")
  set(quiet      "${_EP_QUIET}")

  _ep_get_update_disconnected(update_disconnected)
  if(update_disconnected)
    set(patch_dep download)
  else()
    set(patch_dep update)
  endif()

  # We handle the log setting directly here rather than deferring it to
  # be handled by ExternalProject_Add_Step()
  set(log "${_EP_LOG_PATCH}")
  if(log)
    set(script_filename ${tmp_dir}/${name}-patch-impl.cmake)
    set(log TRUE)
    set(quiet FALSE)  # Already quiet as a result of log being enabled
  else()
    set(script_filename ${tmp_dir}/${name}-patch.cmake)
    set(log FALSE)
  endif()

  if(DEFINED _EP_PATCH_COMMAND)
    set(work_dir "${source_dir}")
    _ep_write_command_script(
      "${script_filename}"
      "${_EP_PATCH_COMMAND}"
      "${work_dir}"
      "${genex_supported}"
      "${quiet}"
      script_does_something
    )
    if(script_does_something)
      set(cmd ${CMAKE_COMMAND} -P ${script_filename})
      if(log)
        _ep_write_log_script(${name} patch "${genex_supported}" cmd)
      endif()
      set(comment "Performing patch step (custom command) for '${name}'")
    else()
      set(cmd)
      set(comment "Skipping patch step (empty custom command) for '${name}'")
    endif()
  else()
    set(cmd)
    set(comment "Skipping patch step (no custom command) for '${name}'")
  endif()

  # Provide variables that can be used later to create a custom command or
  # invoke the step directly
  set(_EPcomment_PATCH         "${comment}"   PARENT_SCOPE)
  set(_EPcommand_PATCH         "${cmd}"       PARENT_SCOPE)
  set(_EPalways_PATCH          FALSE          PARENT_SCOPE)
  set(_EPexcludefrommain_PATCH FALSE          PARENT_SCOPE)
  set(_EPdepends_PATCH         ""             PARENT_SCOPE)
  set(_EPdependees_PATCH       "${patch_dep}" PARENT_SCOPE)

endfunction()

function(_ep_get_file_deps var name)
  set(file_deps)

  get_property(deps TARGET ${name} PROPERTY _EP_DEPENDS)
  foreach(dep IN LISTS deps)
    get_property(dep_type TARGET ${dep} PROPERTY TYPE)
    if(dep_type STREQUAL "UTILITY")
      get_property(is_ep TARGET ${dep} PROPERTY _EP_IS_EXTERNAL_PROJECT)
      if(is_ep)
        _ep_get_step_stampfile(${dep} "done" done_stamp_file)
        list(APPEND file_deps ${done_stamp_file})
      endif()
    endif()
  endforeach()

  set("${var}" "${file_deps}" PARENT_SCOPE)
endfunction()

function(_ep_extract_configure_command var name)
  get_property(cmd_set TARGET ${name} PROPERTY _EP_CONFIGURE_COMMAND SET)
  if(cmd_set)
    get_property(cmd TARGET ${name} PROPERTY _EP_CONFIGURE_COMMAND)
  else()
    get_target_property(cmake_command ${name} _EP_CMAKE_COMMAND)
    if(cmake_command)
      set(cmd "${cmake_command}")
    else()
      set(cmd "${CMAKE_COMMAND}")
    endif()

    get_property(cmake_args TARGET ${name} PROPERTY _EP_CMAKE_ARGS)
    list(APPEND cmd ${cmake_args})

    # If there are any CMAKE_CACHE_ARGS or CMAKE_CACHE_DEFAULT_ARGS,
    # write an initial cache and use it
    get_property(cmake_cache_args TARGET ${name} PROPERTY _EP_CMAKE_CACHE_ARGS)
    get_property(cmake_cache_default_args TARGET ${name} PROPERTY _EP_CMAKE_CACHE_DEFAULT_ARGS)

    set(has_cmake_cache_args 0)
    if(NOT "${cmake_cache_args}" STREQUAL "")
      set(has_cmake_cache_args 1)
    endif()

    set(has_cmake_cache_default_args 0)
    if(NOT "${cmake_cache_default_args}" STREQUAL "")
      set(has_cmake_cache_default_args 1)
    endif()

    get_target_property(cmake_generator ${name} _EP_CMAKE_GENERATOR)
    get_target_property(cmake_generator_instance ${name} _EP_CMAKE_GENERATOR_INSTANCE)
    get_target_property(cmake_generator_platform ${name} _EP_CMAKE_GENERATOR_PLATFORM)
    get_target_property(cmake_generator_toolset ${name} _EP_CMAKE_GENERATOR_TOOLSET)
    if(cmake_generator)
      list(APPEND cmd "-G${cmake_generator}")
      if(cmake_generator_platform)
        list(APPEND cmd "-A${cmake_generator_platform}")
      endif()
      if(cmake_generator_toolset)
        list(APPEND cmd "-T${cmake_generator_toolset}")
      endif()
      if(cmake_generator_instance)
        list(APPEND cmd "-DCMAKE_GENERATOR_INSTANCE:INTERNAL=${cmake_generator_instance}")
      endif()
    else()
      if(CMAKE_EXTRA_GENERATOR)
        list(APPEND cmd "-G${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
      else()
        list(APPEND cmd "-G${CMAKE_GENERATOR}")
        if("${CMAKE_GENERATOR}" MATCHES "Green Hills MULTI")
          set(has_cmake_cache_default_args 1)
          set(cmake_cache_default_args ${cmake_cache_default_args}
            "-DGHS_TARGET_PLATFORM:STRING=${GHS_TARGET_PLATFORM}"
            "-DGHS_PRIMARY_TARGET:STRING=${GHS_PRIMARY_TARGET}"
            "-DGHS_TOOLSET_ROOT:STRING=${GHS_TOOLSET_ROOT}"
            "-DGHS_OS_ROOT:STRING=${GHS_OS_ROOT}"
            "-DGHS_OS_DIR:STRING=${GHS_OS_DIR}"
            "-DGHS_BSP_NAME:STRING=${GHS_BSP_NAME}")
        endif()
      endif()
      if(cmake_generator_platform)
        message(FATAL_ERROR "Option CMAKE_GENERATOR_PLATFORM not allowed without CMAKE_GENERATOR.")
      endif()
      if(CMAKE_GENERATOR_PLATFORM)
        list(APPEND cmd "-A${CMAKE_GENERATOR_PLATFORM}")
      endif()
      if(cmake_generator_toolset)
        message(FATAL_ERROR "Option CMAKE_GENERATOR_TOOLSET not allowed without CMAKE_GENERATOR.")
      endif()
      if(CMAKE_GENERATOR_TOOLSET)
        list(APPEND cmd "-T${CMAKE_GENERATOR_TOOLSET}")
      endif()
      if(cmake_generator_instance)
        message(FATAL_ERROR "Option CMAKE_GENERATOR_INSTANCE not allowed without CMAKE_GENERATOR.")
      endif()
      if(CMAKE_GENERATOR_INSTANCE)
        list(APPEND cmd "-DCMAKE_GENERATOR_INSTANCE:INTERNAL=${CMAKE_GENERATOR_INSTANCE}")
      endif()
    endif()

    if(has_cmake_cache_args OR has_cmake_cache_default_args)
      set(_ep_cache_args_script "<TMP_DIR>/${name}-cache-$<CONFIG>.cmake")
      if(has_cmake_cache_args)
        _ep_command_line_to_initial_cache(script_initial_cache_force "${cmake_cache_args}" 1)
      endif()
      if(has_cmake_cache_default_args)
        _ep_command_line_to_initial_cache(script_initial_cache_default "${cmake_cache_default_args}" 0)
      endif()
      _ep_write_initial_cache(
        ${name}
        "${_ep_cache_args_script}"
        "${script_initial_cache_force}${script_initial_cache_default}"
      )
      list(APPEND cmd "-C${_ep_cache_args_script}")
      _ep_replace_location_tags(${name} _ep_cache_args_script)
      set(_ep_cache_args_script
        "${_ep_cache_args_script}"
        PARENT_SCOPE)
    endif()

    list(APPEND cmd "<SOURCE_DIR><SOURCE_SUBDIR>")
  endif()

  set("${var}" "${cmd}" PARENT_SCOPE)
endfunction()

# TODO: Make sure external projects use the proper compiler
function(_ep_add_configure_command name)
  ExternalProject_Get_Property(${name} binary_dir tmp_dir)

  set(file_deps)
  get_property(configure_handled_by_build TARGET ${name}
               PROPERTY _EP_CONFIGURE_HANDLED_BY_BUILD)
  if(NOT configure_handled_by_build)
    # Depend on other external projects (file-level)
    _ep_get_file_deps(file_deps ${name})
  endif()

  _ep_extract_configure_command(cmd ${name})

  # If anything about the configure command changes, (command itself, cmake
  # used, cmake args or cmake generator) then re-run the configure step.
  # Fixes issue https://gitlab.kitware.com/cmake/cmake/-/issues/10258
  #
  configure_file(
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ExternalProject/cfgcmd.txt.in
    ${tmp_dir}/${name}-cfgcmd.txt
    @ONLY
  )
  list(APPEND file_deps ${tmp_dir}/${name}-cfgcmd.txt)
  list(APPEND file_deps ${_ep_cache_args_script})

  get_property(log TARGET ${name} PROPERTY _EP_LOG_CONFIGURE)
  if(log)
    set(log LOG 1)
  else()
    set(log "")
  endif()

  get_property(uses_terminal TARGET ${name} PROPERTY
    _EP_USES_TERMINAL_CONFIGURE)
  if(uses_terminal)
    set(uses_terminal USES_TERMINAL 1)
  else()
    set(uses_terminal "")
  endif()

  set(__cmdQuoted)
  foreach(__item IN LISTS cmd)
    string(APPEND __cmdQuoted " [==[${__item}]==]")
  endforeach()
  cmake_language(EVAL CODE "
    ExternalProject_Add_Step(${name} configure
      INDEPENDENT FALSE
      COMMAND ${__cmdQuoted}
      WORKING_DIRECTORY \${binary_dir}
      DEPENDEES patch
      DEPENDS \${file_deps}
      ${log}
      ${uses_terminal}
      )"
  )
endfunction()


function(_ep_add_build_command name)
  ExternalProject_Get_Property(${name} binary_dir)

  set(file_deps)
  get_property(configure_handled_by_build TARGET ${name}
               PROPERTY _EP_CONFIGURE_HANDLED_BY_BUILD)
  if(configure_handled_by_build)
    # Depend on other external projects (file-level)
    _ep_get_file_deps(file_deps ${name})
  endif()

  get_property(cmd_set TARGET ${name} PROPERTY _EP_BUILD_COMMAND SET)
  if(cmd_set)
    get_property(cmd TARGET ${name} PROPERTY _EP_BUILD_COMMAND)
  else()
    _ep_get_build_command(${name} BUILD cmd)
  endif()

  get_property(log TARGET ${name} PROPERTY _EP_LOG_BUILD)
  if(log)
    set(log LOG 1)
  else()
    set(log "")
  endif()

  get_property(uses_terminal TARGET ${name} PROPERTY
    _EP_USES_TERMINAL_BUILD)
  if(uses_terminal)
    set(uses_terminal USES_TERMINAL 1)
  else()
    set(uses_terminal "")
  endif()

  get_property(build_always TARGET ${name} PROPERTY _EP_BUILD_ALWAYS)
  if(build_always)
    set(always 1)
  else()
    set(always 0)
  endif()

  get_property(build_byproducts TARGET ${name} PROPERTY _EP_BUILD_BYPRODUCTS)

  set(__cmdQuoted)
  foreach(__item IN LISTS cmd)
    string(APPEND __cmdQuoted " [==[${__item}]==]")
  endforeach()
  cmake_language(EVAL CODE "
    ExternalProject_Add_Step(${name} build
      INDEPENDENT FALSE
      COMMAND ${__cmdQuoted}
      BYPRODUCTS \${build_byproducts}
      WORKING_DIRECTORY \${binary_dir}
      DEPENDEES configure
      DEPENDS \${file_deps}
      ALWAYS \${always}
      ${log}
      ${uses_terminal}
      )"
  )
endfunction()


function(_ep_add_install_command name)
  ExternalProject_Get_Property(${name} binary_dir)

  get_property(cmd_set TARGET ${name} PROPERTY _EP_INSTALL_COMMAND SET)
  if(cmd_set)
    get_property(cmd TARGET ${name} PROPERTY _EP_INSTALL_COMMAND)
  else()
    _ep_get_build_command(${name} INSTALL cmd)
  endif()

  get_property(log TARGET ${name} PROPERTY _EP_LOG_INSTALL)
  if(log)
    set(log LOG 1)
  else()
    set(log "")
  endif()

  get_property(uses_terminal TARGET ${name} PROPERTY
    _EP_USES_TERMINAL_INSTALL)
  if(uses_terminal)
    set(uses_terminal USES_TERMINAL 1)
  else()
    set(uses_terminal "")
  endif()

  set(__cmdQuoted)
  foreach(__item IN LISTS cmd)
    string(APPEND __cmdQuoted " [==[${__item}]==]")
  endforeach()
  cmake_language(EVAL CODE "
    ExternalProject_Add_Step(${name} install
      INDEPENDENT FALSE
      COMMAND ${__cmdQuoted}
      WORKING_DIRECTORY \${binary_dir}
      DEPENDEES build
      ${log}
      ${uses_terminal}
      )"
  )
endfunction()


function(_ep_add_test_command name)
  ExternalProject_Get_Property(${name} binary_dir)

  get_property(before TARGET ${name} PROPERTY _EP_TEST_BEFORE_INSTALL)
  get_property(after TARGET ${name} PROPERTY _EP_TEST_AFTER_INSTALL)
  get_property(exclude TARGET ${name} PROPERTY _EP_TEST_EXCLUDE_FROM_MAIN)
  get_property(cmd_set TARGET ${name} PROPERTY _EP_TEST_COMMAND SET)

  # Only actually add the test step if one of the test related properties is
  # explicitly set. (i.e. the test step is omitted unless requested...)
  #
  if(cmd_set OR before OR after OR exclude)
    if(cmd_set)
      get_property(cmd TARGET ${name} PROPERTY _EP_TEST_COMMAND)
    else()
      _ep_get_build_command(${name} TEST cmd)
    endif()

    if(before)
      set(dependees_args DEPENDEES build)
    else()
      set(dependees_args DEPENDEES install)
    endif()

    if(exclude)
      set(dependers_args "")
      set(exclude_args EXCLUDE_FROM_MAIN 1)
    else()
      if(before)
        set(dependers_args DEPENDERS install)
      else()
        set(dependers_args "")
      endif()
      set(exclude_args "")
    endif()

    get_property(log TARGET ${name} PROPERTY _EP_LOG_TEST)
    if(log)
      set(log LOG 1)
    else()
      set(log "")
    endif()

    get_property(uses_terminal TARGET ${name} PROPERTY
      _EP_USES_TERMINAL_TEST)
    if(uses_terminal)
      set(uses_terminal USES_TERMINAL 1)
    else()
      set(uses_terminal "")
    endif()

    set(__cmdQuoted)
    foreach(__item IN LISTS cmd)
      string(APPEND __cmdQuoted " [==[${__item}]==]")
    endforeach()
    cmake_language(EVAL CODE "
      ExternalProject_Add_Step(${name} test
        INDEPENDENT FALSE
        COMMAND ${__cmdQuoted}
        WORKING_DIRECTORY \${binary_dir}
        ${dependees_args}
        ${dependers_args}
        ${exclude_args}
        ${log}
        ${uses_terminal}
        )"
    )
  endif()
endfunction()

macro(_ep_get_add_keywords out_var)
  set(${out_var}
    #
    # Directory options
    #
    PREFIX
    TMP_DIR
    STAMP_DIR
    LOG_DIR
    DOWNLOAD_DIR
    SOURCE_DIR
    BINARY_DIR
    INSTALL_DIR
    #
    # Download step options
    #
    DOWNLOAD_COMMAND
    #
    URL
    URL_HASH
    URL_MD5
    DOWNLOAD_NAME
    DOWNLOAD_NO_EXTRACT
    DOWNLOAD_NO_PROGRESS
    TIMEOUT
    INACTIVITY_TIMEOUT
    HTTP_USERNAME
    HTTP_PASSWORD
    HTTP_HEADER
    TLS_VERIFY     # Also used for git clone operations
    TLS_CAINFO
    NETRC
    NETRC_FILE
    #
    GIT_REPOSITORY
    GIT_TAG
    GIT_REMOTE_NAME
    GIT_SUBMODULES
    GIT_SUBMODULES_RECURSE
    GIT_SHALLOW
    GIT_PROGRESS
    GIT_CONFIG
    GIT_REMOTE_UPDATE_STRATEGY
    #
    SVN_REPOSITORY
    SVN_REVISION
    SVN_USERNAME
    SVN_PASSWORD
    SVN_TRUST_CERT
    #
    HG_REPOSITORY
    HG_TAG
    #
    CVS_REPOSITORY
    CVS_MODULE
    CVS_TAG
    #
    # Update step options
    #
    UPDATE_COMMAND
    UPDATE_DISCONNECTED
    #
    # Patch step options
    #
    PATCH_COMMAND
    #
    # Configure step options
    #
    CONFIGURE_COMMAND
    CMAKE_COMMAND
    CMAKE_GENERATOR
    CMAKE_GENERATOR_PLATFORM
    CMAKE_GENERATOR_TOOLSET
    CMAKE_GENERATOR_INSTANCE
    CMAKE_ARGS
    CMAKE_CACHE_ARGS
    CMAKE_CACHE_DEFAULT_ARGS
    SOURCE_SUBDIR
    CONFIGURE_HANDLED_BY_BUILD
    #
    # Build step options
    #
    BUILD_COMMAND
    BUILD_IN_SOURCE
    BUILD_ALWAYS
    BUILD_BYPRODUCTS
    #
    # Install step options
    #
    INSTALL_COMMAND
    #
    # Test step options
    #
    TEST_COMMAND
    TEST_BEFORE_INSTALL
    TEST_AFTER_INSTALL
    TEST_EXCLUDE_FROM_MAIN
    #
    # Logging options
    #
    LOG_DOWNLOAD
    LOG_UPDATE
    LOG_PATCH
    LOG_CONFIGURE
    LOG_BUILD
    LOG_INSTALL
    LOG_TEST
    LOG_MERGED_STDOUTERR
    LOG_OUTPUT_ON_FAILURE
    #
    # Terminal access options
    #
    USES_TERMINAL_DOWNLOAD
    USES_TERMINAL_UPDATE
    USES_TERMINAL_CONFIGURE
    USES_TERMINAL_BUILD
    USES_TERMINAL_INSTALL
    USES_TERMINAL_TEST
    #
    # Target options
    #
    DEPENDS
    EXCLUDE_FROM_ALL
    STEP_TARGETS
    INDEPENDENT_STEP_TARGETS
    #
    # Miscellaneous options
    #
    LIST_SEPARATOR
  )
endmacro()


# Internal function called by FetchContent to populate immediately.
# It only executes steps up to and including "patch". It takes the same
# arguments as ExternalProject_Add() plus one additional argument: QUIET.
#
# Not to be used outside of CMake.
#
function(_ep_do_preconfigure_steps_now name)

  cmake_policy(GET CMP0097 _EP_CMP0097
    PARENT_SCOPE # undocumented, do not use outside of CMake
  )

  set(genex_supported FALSE)

  _ep_get_add_keywords(keywords)
  _ep_parse_arguments_to_vars("${keywords};QUIET" ${name} _EP_ "${ARGN}")

  _ep_get_update_disconnected(update_disconnected)

  _ep_prepare_directories(${name})
  _ep_prepare_download(${name} ${genex_supported})
  _ep_prepare_update(${name} ${genex_supported})
  _ep_prepare_patch(${name} ${genex_supported})

  set(stamp_dir "${_EP_STAMP_DIR}")
  set(tmp_dir   "${_EP_TMP_DIR}")

  # Once any step has to run, all later steps have to be run too
  set(need_to_run FALSE)
  foreach(step IN ITEMS download update patch)
    if(update_disconnected AND "${step}" STREQUAL "update")
      continue()
    endif()

    string(TOUPPER "${step}" STEP)
    if("${_EPcommand_${STEP}}" STREQUAL "")
      continue()
    endif()

    set(stamp_file "${stamp_dir}/${name}-${step}")
    set(script_file ${tmp_dir}/${name}-${step}.cmake)

    if(NOT EXISTS ${stamp_file})
      set(need_to_run TRUE)
    endif()

    if(NOT need_to_run)
      foreach(dep_file ${script_file} ${_EPdepends_${STEP}})
        # IS_NEWER_THAN is also true if the timestamps are the same. On some
        # file systems, we only have second resolution timestamps and the
        # likelihood of having the same timestamp is high. Use the negative
        # form to ensure we actually get a true "is newer than" test.
        if(NOT EXISTS ${dep_file} OR
           NOT ${stamp_file} IS_NEWER_THAN ${dep_file})
          set(need_to_run TRUE)
          break()
        endif()
      endforeach()
    endif()

    if(need_to_run)
      include(${script_file})
      file(TOUCH ${stamp_file})
    endif()
  endforeach()

  if("${_EP_DOWNLOAD_NO_EXTRACT}")
    file(COPY "${_EP_DOWNLOADED_FILE}" DESTINATION "${_EP_SOURCE_DIR}")
  endif()

endfunction()

function(ExternalProject_Add name)
  cmake_policy(GET CMP0097 _EP_CMP0097
    PARENT_SCOPE # undocumented, do not use outside of CMake
    )
  cmake_policy(GET CMP0114 cmp0114
    PARENT_SCOPE # undocumented, do not use outside of CMake
    )
  if(CMAKE_XCODE_BUILD_SYSTEM VERSION_GREATER_EQUAL 12 AND NOT cmp0114 STREQUAL "NEW")
    message(AUTHOR_WARNING
      "Policy CMP0114 is not set to NEW.  "
      "In order to support the Xcode \"new build system\", "
      "this project must be updated to set policy CMP0114 to NEW."
      "\n"
      "Since CMake is generating for the Xcode \"new build system\", "
      "ExternalProject_Add will use policy CMP0114's NEW behavior anyway, "
      "but the generated build system may not match what the project intends."
      )
    set(cmp0114 "NEW")
  endif()

  set(genex_supported TRUE)

  _ep_get_add_keywords(keywords)
  _ep_parse_arguments_to_vars("${keywords}" ${name} _EP_ "${ARGN}")
  _ep_prepare_directories(${name})
  _ep_prepare_download(${name} ${genex_supported})
  _ep_prepare_update(${name} ${genex_supported})
  _ep_prepare_patch(${name} ${genex_supported})

  _ep_get_configuration_subdir_suffix(cfgdir)

  # Add a custom target for the external project.
  set(cmf_dir ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles)
  _ep_get_complete_stampfile(${name} complete_stamp_file)

  cmake_policy(PUSH)
  if(cmp0114 STREQUAL "NEW")
    # To implement CMP0114 NEW behavior with Makefile generators,
    # we need CMP0113 NEW behavior.
    cmake_policy(SET CMP0113 NEW)
  endif()
  # The "ALL" option to add_custom_target just tells it to not set the
  # EXCLUDE_FROM_ALL target property. Later, if the EXCLUDE_FROM_ALL
  # argument was passed, we explicitly set it for the target.
  add_custom_target(${name} ALL DEPENDS ${complete_stamp_file})
  cmake_policy(POP)
  set_property(TARGET ${name} PROPERTY _EP_IS_EXTERNAL_PROJECT 1)
  set_property(TARGET ${name} PROPERTY LABELS ${name})
  set_property(TARGET ${name} PROPERTY FOLDER "ExternalProjectTargets/${name}")

  set_property(TARGET ${name} PROPERTY _EP_CMP0114 "${cmp0114}")

  # Transfer the arguments to the target as target properties. These are
  # read by the various steps, potentially from different scopes.
  foreach(key IN LISTS keywords ITEMS DOWNLOADED_FILE)
    if(DEFINED _EP_${key})
      set_property(TARGET ${name} PROPERTY _EP_${key} "${_EP_${key}}")
    endif()
  endforeach()

  _ep_get_step_stampfile(${name} "done" done_stamp_file)
  _ep_get_step_stampfile(${name} "install" install_stamp_file)

  if(arg_EXCLUDE_FROM_ALL)
    set_property(TARGET ${name} PROPERTY EXCLUDE_FROM_ALL TRUE)
  endif()

  # The 'complete' step depends on all other steps and creates a
  # 'done' mark.  A dependent external project's 'configure' step
  # depends on the 'done' mark so that it rebuilds when this project
  # rebuilds.  It is important that 'done' is not the output of any
  # custom command so that CMake does not propagate build rules to
  # other external project targets, which may cause problems during
  # parallel builds.  However, the Ninja generator needs to see the entire
  # dependency graph, and can cope with custom commands belonging to
  # multiple targets, so we add the 'done' mark as an output for Ninja only.
  set(complete_outputs ${complete_stamp_file})
  if(${CMAKE_GENERATOR} MATCHES "Ninja")
    set(complete_outputs ${complete_outputs} ${done_stamp_file})
  endif()

  add_custom_command(
    OUTPUT ${complete_outputs}
    COMMENT "Completed '${name}'"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${cmf_dir}${cfgdir}
    COMMAND ${CMAKE_COMMAND} -E touch ${complete_stamp_file}
    COMMAND ${CMAKE_COMMAND} -E touch ${done_stamp_file}
    DEPENDS ${install_stamp_file}
    VERBATIM
    )


  # Depend on other external projects (target-level).
  get_property(deps TARGET ${name} PROPERTY _EP_DEPENDS)
  foreach(arg IN LISTS deps)
    add_dependencies(${name} ${arg})
  endforeach()

  # Set up custom build steps based on the target properties.
  # Each step depends on the previous one.
  #
  # The target depends on the output of the final step.
  # (Already set up above in the DEPENDS of the add_custom_target command.)
  #
  _ep_add_preconfigure_command(${name} mkdir)
  _ep_add_preconfigure_command(${name} download)
  _ep_add_preconfigure_command(${name} update)
  _ep_add_preconfigure_command(${name} patch)
  _ep_add_configure_command(${name})
  _ep_add_build_command(${name})
  _ep_add_install_command(${name})

  # Test is special in that it might depend on build, or it might depend
  # on install.
  #
  _ep_add_test_command(${name})
endfunction()

cmake_policy(POP)
