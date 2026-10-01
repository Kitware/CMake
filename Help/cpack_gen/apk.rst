CPack APK Generator
-------------------

.. versionadded:: 4.5

The CPack ``APK`` generator packages an application for Android as an
`APK`_ file.  It drives the Android SDK build tools (``aapt2``, ``d8``,
``zipalign`` and ``apksigner``) and the JDK directly, so neither Gradle
nor a Java build system of any kind is required.

.. _`APK`: https://developer.android.com/guide/components/fundamentals

The generator is meant for applications whose Java code is shipped with
the project or available as ``.jar`` or ``.aar`` files, locally or in
Maven repositories.  Nothing is downloaded unless the project lists
Maven dependencies in :variable:`CPACK_APK_MAVEN_DEPENDENCIES`; see
`Maven dependencies`_.  Projects whose builds need more of Gradle should
keep using Gradle.

How the staging tree is packaged
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

CPack installs the project into a staging tree and the generator turns
that tree into the contents of the APK.  Most files are added to the APK
at the same relative path they were installed to, so a native library
installed to ``lib/arm64-v8a/`` ends up at ``lib/arm64-v8a/`` inside the
APK.  The following entries of the staging tree are treated specially
instead, because the build tools consume them:

``AndroidManifest.xml``
  Used as the application manifest, unless
  :variable:`CPACK_APK_MANIFEST` points elsewhere.

``res/``
  Compiled and linked by ``aapt2``, unless
  :variable:`CPACK_APK_RESOURCE_DIRECTORIES` is set.

``assets/``
  Added to the APK by ``aapt2``, unless
  :variable:`CPACK_APK_ASSET_DIRECTORIES` is set.

``*.jar`` and ``*.aar``
  Converted to Dalvik bytecode by ``d8`` and added as ``classes.dex``.
  Archives below ``res/`` or ``assets/`` are left alone: those are
  resources of the application, not code.  See `Android archives`_ for
  what is taken from an ``.aar``.

``AndroidManifest.xml``, ``res/`` and ``assets/`` are never copied into
the APK as they are, because ``aapt2`` puts its own compiled versions of
them there.  Overriding the variables above changes what ``aapt2`` is
given, not whether the staged copies are packaged.

A minimal example:

.. code-block:: cmake

  install(TARGETS my_application
          LIBRARY DESTINATION lib/${CMAKE_ANDROID_ARCH_ABI})
  install(FILES AndroidManifest.xml DESTINATION .)
  install(DIRECTORY res DESTINATION .)

  set(CPACK_GENERATOR APK)
  set(CPACK_APK_JAVA_SOURCE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/java")
  include(CPack)

Like the Android Gradle plugin, the generator packages the shared C++
runtime of the NDK, ``libc++_shared.so``, into ``lib/<abi>/`` for the ABIs
whose native libraries are linked to it, as those built with
:variable:`CMAKE_ANDROID_STL_TYPE` set to ``c++_shared`` are, unless the
project installs one itself.  The libraries linked to it are those whose
dynamic section, as printed by ``<readelf> -d <library>``, lists it as
``NEEDED``.  :variable:`CPACK_READELF_EXECUTABLE`, which defaults to
``CMAKE_READELF``, the ``llvm-readelf`` of the NDK when cross-compiling for
Android, is used for that; when it is not set, the ``llvm-readelf`` of the
NDK is, found like the ``llvm-strip`` of
:variable:`CPACK_APK_STRIP_EXECUTABLE`.  The runtime is taken from the
sysroot of that same NDK, or else of the NDK the ``llvm-strip`` or the
``llvm-readelf`` belongs to.  The generator warns when it finds no
``readelf`` to check the libraries with, or no runtime to package.

The resulting APK is signed, so it can be installed with ``adb install``
right away.  By default it is signed with the same debug keystore the
rest of the Android tooling uses, ``debug.keystore`` in the directory
named by the ``ANDROID_USER_HOME`` environment variable or else in
``~/.android``, which is created if it does not exist yet.  On Windows,
``~`` is the user's profile directory, ``%USERPROFILE%``.  Use
:variable:`CPACK_APK_KEYSTORE` and the related variables to sign with a
key of your own.

Multiple ABIs
^^^^^^^^^^^^^

A build tree targets a single Android ABI.  To package the native code
of several ABIs into one APK, use a superbuild: a project that builds the
application once per ABI with :module:`ExternalProject`, from the same
sources, and lists the build trees in
:variable:`CPACK_INSTALL_CMAKE_PROJECTS`.  The superbuild names the build
trees itself and brings all of them up to date whenever it is built, so
the ABIs are always packaged from the same revision of the sources.  CPack
installs each build tree into the same staging tree, so the native
libraries end up side by side in ``lib/<abi>/``:

.. code-block:: cmake

  cmake_minimum_required(VERSION 4.5)
  project(MySuperBuild LANGUAGES NONE)

  include(ExternalProject)

  # The NDK is given with -DANDROID_NDK=<path>.
  foreach(abi IN ITEMS arm64-v8a x86_64)
    set(binary_dir "${CMAKE_CURRENT_BINARY_DIR}/${abi}")
    ExternalProject_Add(app-${abi}
      SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/app"
      BINARY_DIR "${binary_dir}"
      CMAKE_ARGS
        "-DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake"
        -DANDROID_ABI=${abi}
        -DCMAKE_BUILD_TYPE=$<CONFIG>
      BUILD_ALWAYS TRUE
      INSTALL_COMMAND ""
    )
    list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${binary_dir};MyApp;ALL;/")
  endforeach()

  set(CPACK_GENERATOR APK)
  set(CPACK_APK_NDK_ROOT "${ANDROID_NDK}")
  set(CPACK_APK_JAVA_SOURCE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/app/java")
  include(CPack)

The package is then made with ``cmake --build`` followed by ``cpack`` in
the build tree of the superbuild.  It is the configuration of the
superbuild that CPack reads, so the ``CPACK_APK_*`` variables are set
there rather than in the application.  Since the superbuild does not
cross-compile, :variable:`CPACK_APK_NDK_ROOT` tells where to take the
``llvm-strip``, the ``llvm-readelf`` and the shared C++ runtime from,
unless the generator finds the NDK otherwise.

The native libraries have to be installed into ``lib/<abi>/``, as in the
example of `How the staging tree is packaged`_.  The files the build
trees have in common, such as the manifest and the resources, are taken
from the one installed last.  The shared C++ runtime and the native
libraries of Android archives are packaged for each of the ABIs.

Android archives
^^^^^^^^^^^^^^^^

From an ``.aar``, the generator takes:

* ``classes.jar`` and ``libs/*.jar``, which are converted to Dalvik
  bytecode.  The ``R`` class of the archive's package, which the archive
  does not ship, is generated along with the application's.
* ``res/`` and ``assets/``, which the application's own resources and
  assets override.
* ``jni/<abi>/*.so``, packaged as ``lib/<abi>/`` and stripped like the
  application's own libraries, see :variable:`CPACK_APK_STRIP_EXECUTABLE`.
  Only the ABIs the application ships native code for, or
  :variable:`CPACK_APK_ABI`, are packaged; all of them when the
  application has no native code.
* The permissions, features, ``<queries>``, components, ``<meta-data>``
  and libraries declared by ``AndroidManifest.xml``, which are merged into
  the application manifest.  An element the application manifest, or an
  archive before it, already declares under the same ``android:name`` is
  left out, except that the child elements of a component, such as the
  ``<meta-data>`` of the ``androidx.startup.InitializationProvider`` that
  several AndroidX libraries declare, are merged into the one declared
  first, unless it is marked ``tools:node="replace"``.  Children declared
  under the same ``android:name`` are again left out.  Beyond that, the
  merge rules of the Gradle manifest merger are not applied: ``tools:``
  attributes are dropped, except that an
  element marked ``tools:node="remove"``, at any depth, is not merged.
  Such an element at the top level
  of the application manifest is not packaged either, and keeps the
  elements of the archives declared under the same ``android:name`` out.

Maven dependencies
^^^^^^^^^^^^^^^^^^

Libraries such as those of AndroidX are published to Maven repositories
rather than installed with the SDK.  Listing them in
:variable:`CPACK_APK_MAVEN_DEPENDENCIES` makes the generator download
them and the libraries they depend on at packaging time, and add them
to :variable:`CPACK_APK_JARS`:

.. code-block:: cmake

  set(CPACK_APK_MAVEN_DEPENDENCIES androidx.core:core:1.17.0)

This is opt-in: when the variable is not set, no repository is
contacted.  Each file that is downloaded is reported, and downloaded
files are kept in :variable:`CPACK_APK_MAVEN_CACHE_DIRECTORY`, so that
later runs work offline.  Dependencies are resolved like Gradle resolves
the runtime classpath of an Android application:

* The Gradle module metadata (``.module`` file) of an artifact is used
  when the repository has it, which is the case for AndroidX, Kotlin and
  most libraries built with Gradle.  Its variant for the Java runtime is
  selected, preferring the one for Android over the one for the JVM, and
  the one for the release build type over the one for debug.  A variant
  published as a separate module, as Kotlin Multiplatform libraries do, is
  followed there.  The files are checked against the SHA-256 hashes it
  lists.
* Otherwise the POM is used, for its dependencies of the ``compile`` and
  ``runtime`` scopes that are not optional, and for the ``packaging`` of
  the artifact.  The dependencies of build plugins and of profiles are
  not used.  The file is checked against the SHA-1 hash published next to
  it, if any.  Parent POMs and imported BOMs are not read, so a version
  defined in one of them is an error.  The ``project.groupId``,
  ``project.artifactId`` and ``project.version`` properties, and those of
  the POM's own ``<properties>``, are replaced.
* When an artifact is required in several versions, the highest one is
  used, in the order of Gradle: ``1.0-alpha01`` is lower than
  ``1.0-rc01``, which is lower than ``1.0``.  As with Gradle, the versions
  that an artifact requires stop counting once a higher version of it is
  selected instead.  Dependency constraints,
  including those of the platforms that Gradle module metadata depends on,
  such as BOMs, can raise the version of an artifact that is required
  anyway, or give one to a dependency that has none.  It is an error when
  no version is given.  Version ranges are not supported, except for a
  single version in brackets, like ``[1.2.3]``.
* Exclusions of transitive dependencies, in POMs or in Gradle module
  metadata, leave an artifact out when every path to it excludes it.

The Android SDK
^^^^^^^^^^^^^^^

The SDK is looked for in :variable:`CPACK_APK_SDK_ROOT`, then in the
``ANDROID_HOME`` and ``ANDROID_SDK_ROOT`` environment variables, and
finally next to the NDK named by :variable:`CPACK_APK_NDK_ROOT` or by the
``ANDROID_NDK_ROOT``, ``ANDROID_NDK_HOME`` and ``ANDROID_NDK``
environment variables.  When :variable:`CPACK_APK_SDK_ROOT` is set, it
must hold the SDK: the other locations are not considered.  When the
project is cross-compiled for Android, :variable:`CPACK_APK_NDK_ROOT`
defaults to :variable:`CMAKE_ANDROID_NDK`.

A JDK is required as well: ``d8`` and ``apksigner`` are Java programs,
and ``jar`` and ``keytool`` are used directly.  Java sources are compiled
against the Java library of the Android platform rather than the one of
the JDK, using a system image built from the platform's
``core-for-system-modules.jar`` with ``jmod`` and ``jlink``.  Platforms
older than API level 30 do not provide that file, in which case the Java
library of the JDK is used.

Variables Specific to CPack APK Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Relative paths given to the variables below are relative to the
directory cpack runs in.

.. variable:: CPACK_APK_SDK_ROOT

  Root directory of the Android SDK.

  :Default: Detected as described above

.. variable:: CPACK_APK_JAVA_HOME

  Root directory of the JDK.  The Java tools are taken from it, and the
  ``JAVA_HOME`` environment variable is set to it for the SDK build
  tools.

  :Default: The ``JAVA_HOME`` environment variable, then on macOS the
            output of ``/usr/libexec/java_home``, and finally the JDK
            the ``javac`` found on the ``PATH`` belongs to

.. variable:: CPACK_APK_NDK_ROOT

  Root directory of the Android NDK.  Used to locate the SDK it is
  installed in, the ``llvm-strip`` of
  :variable:`CPACK_APK_STRIP_EXECUTABLE`, and the ``libc++_shared.so``
  packaged for the native libraries linked to it.

  :Default: :variable:`CMAKE_ANDROID_NDK` when cross-compiling for Android,
            otherwise unset

.. variable:: CPACK_APK_BUILD_TOOLS_VERSION

  Version of the SDK build tools to use, e.g. ``35.0.0``.

  :Default: The highest version installed in ``<sdk>/build-tools``

.. variable:: CPACK_APK_COMPILE_SDK_VERSION

  API level whose ``android.jar`` the resources and the Java sources are
  compiled against, e.g. ``35``.

  :Default: The highest API level installed in ``<sdk>/platforms``

.. variable:: CPACK_APK_MIN_SDK_VERSION

  Minimum API level the application supports.  Written into the
  ``<uses-sdk>`` element of the manifest, replacing the value it
  declares.  Either way, ``d8`` targets the resulting API level.

  :Default: Unset

.. variable:: CPACK_APK_TARGET_SDK_VERSION

  API level the application targets.  Written into the ``<uses-sdk>``
  element of the manifest, replacing the value it declares.

  :Default: The value declared by the manifest, otherwise
            :variable:`CPACK_APK_COMPILE_SDK_VERSION`

.. variable:: CPACK_APK_ABI

  Android ABI of the native code, e.g. ``arm64-v8a``.  Shared libraries
  installed directly into ``lib/`` are moved to ``lib/<abi>/`` inside the
  APK, which is where Android expects them.  Libraries already installed
  into an ABI sub-directory are left alone.

  :Default: :variable:`CMAKE_ANDROID_ARCH_ABI` when cross-compiling for
            Android, otherwise unset

.. variable:: CPACK_APK_STRIP_EXECUTABLE

  Tool that strips the debug symbols from the native libraries packaged in
  ``lib/<abi>/``, those of Android archives included, as the Android Gradle
  plugin does.  It is run as ``<strip> --strip-unneeded -o <output>
  <input>``.  Files that are not ELF are packaged as they are.  When no
  tool is found, the libraries are packaged as they are, with a warning;
  when stripping a library fails, that one is, with a warning as well.
  A tool set explicitly that does not exist is an error.

  :Default: ``CMAKE_STRIP`` when cross-compiling for Android, otherwise
            the ``llvm-strip`` of the
            NDK of :variable:`CPACK_APK_NDK_ROOT`, of the ``ANDROID_NDK_ROOT``,
            ``ANDROID_NDK_HOME`` or ``ANDROID_NDK`` environment variables,
            or else of the highest version installed in ``<sdk>/ndk``

.. variable:: CPACK_APK_KEEP_DEBUG_SYMBOLS

  List of file name patterns, in which ``*`` and ``?`` are wildcards, of
  the native libraries to package with their debug symbols, like the
  ``keepDebugSymbols`` packaging option of the Android Gradle plugin.  For
  example, ``*.so`` keeps them in all libraries.

  :Default: Unset

.. variable:: CPACK_APK_MANIFEST

  Path to the ``AndroidManifest.xml`` to package.

  :Default: ``AndroidManifest.xml`` at the top of the staging tree

.. variable:: CPACK_APK_PACKAGE

  Package of the application, e.g. ``com.example.app``.  Written into the
  ``package`` attribute of the manifest, replacing the value it declares.
  Relative class names of the manifest resolve against it.  This is what
  Gradle projects set as the ``namespace`` in their build script, leaving
  it out of the manifest.

  :Default: The value declared by the manifest, which must then declare one

.. variable:: CPACK_APK_MANIFEST_PLACEHOLDERS

  List of ``<name>=<value>`` entries.  Each ``${<name>}`` in the manifest
  is replaced by the corresponding value before the manifest is compiled.
  This is how the Gradle-style placeholders found in manifest templates,
  most notably ``${applicationId}``, can be resolved.  Unless listed here,
  ``${applicationId}`` is the ``package`` of the manifest.

  :Default: Unset

.. variable:: CPACK_APK_RESOURCE_DIRECTORIES

  List of Android resource directories to compile into the APK.  They are
  merged in order, with later directories overlaying earlier ones.  The
  resources of Android archives come before all of them.

  :Default: ``res`` at the top of the staging tree, if present

.. variable:: CPACK_APK_ASSET_DIRECTORIES

  List of directories whose contents are packaged as Android assets.
  They are merged in order, with files of later directories replacing
  those of earlier ones.  The assets of Android archives come before all
  of them.

  :Default: ``assets`` at the top of the staging tree, if present

.. variable:: CPACK_APK_JAVA_SOURCE_DIRECTORIES

  List of directories to compile ``.java`` files from.  They are searched
  recursively.  The generated ``R`` classes are compiled along with them.

  :Default: Unset

.. variable:: CPACK_APK_JAVA_VERSION

  Java release to compile the Java sources for, passed to ``javac`` as its
  ``-source`` and ``-target`` options.  Left to itself, ``javac`` targets
  the release of the JDK, whose class files the ``d8`` of older build
  tools cannot read.  Set this to an empty string to pass neither option.
  It is not used when :variable:`CPACK_APK_JAVAC_EXTRA_OPTIONS` holds one
  of the ``--release``, ``-source`` or ``-target`` options.

  :Default: ``11``

.. variable:: CPACK_APK_JARS

  List of additional ``.jar`` and ``.aar`` files to convert to Dalvik
  bytecode and to compile the Java sources against.  Archives found in
  the staging tree, and those of
  :variable:`CPACK_APK_MAVEN_DEPENDENCIES`, are added to this list.

  :Default: Unset

.. variable:: CPACK_APK_MAVEN_DEPENDENCIES

  List of Maven artifacts to download, with the artifacts they depend
  on, and to add to :variable:`CPACK_APK_JARS`, each given as
  ``<group>:<artifact>:<version>``.  See `Maven dependencies`_.

  :Default: Unset: nothing is downloaded

.. variable:: CPACK_APK_MAVEN_REPOSITORIES

  List of Maven repositories to download
  :variable:`CPACK_APK_MAVEN_DEPENDENCIES` from, tried in order for each
  file.  An entry is a URL, or else a path to a local directory laid out
  as a Maven repository.  The local repository of Maven itself,
  ``~/.m2/repository``, is one, so listing it first reuses the artifacts
  Maven has already downloaded.  Maven does not keep Gradle module
  metadata there, which is then still downloaded from the repositories
  listed after it:

  .. code-block:: cmake

    set(CPACK_APK_MAVEN_REPOSITORIES
      "$ENV{HOME}/.m2/repository"
      https://maven.google.com
      https://repo.maven.apache.org/maven2
      )

  On Windows, the user's home directory is ``$ENV{USERPROFILE}``.  A
  ``~`` in an entry is not expanded.

  :Default: ``https://maven.google.com`` and
            ``https://repo.maven.apache.org/maven2``

.. variable:: CPACK_APK_MAVEN_CACHE_DIRECTORY

  Directory to download :variable:`CPACK_APK_MAVEN_DEPENDENCIES` to.
  Files found there are not downloaded again.

  :Default: ``_CPack_Packages/maven`` in
            :variable:`CPACK_PACKAGE_DIRECTORY`

.. variable:: CPACK_APK_VERSION_CODE

  Integer version code of the application.  Written into the manifest,
  replacing the value it declares.

  :Default: The value declared by the manifest, otherwise ``1``

.. variable:: CPACK_APK_VERSION_NAME

  User visible version of the application.  Written into the manifest,
  replacing the value it declares.

  :Default: The value declared by the manifest, otherwise
            :variable:`CPACK_PACKAGE_VERSION`

.. variable:: CPACK_APK_NO_COMPRESS_EXTENSIONS

  List of file name extensions, leading dot included, to store in the APK
  uncompressed.  Android maps uncompressed native libraries straight out
  of the APK, which is required when the manifest sets
  ``android:extractNativeLibs`` to ``false``.

  :Default: ``.so``

.. variable:: CPACK_APK_SIGN

  Boolean controlling whether the APK is signed.  An unsigned APK cannot
  be installed on a device.

  :Default: ``TRUE``

.. variable:: CPACK_APK_KEYSTORE

  Path to the Java keystore holding the signing key.

  :Default: The debug keystore described above, generated if missing

.. variable:: CPACK_APK_KEYSTORE_PASSWORD

  Password of the keystore.

  :Default: ``android``, together with the default keystore

.. variable:: CPACK_APK_KEY_ALIAS

  Alias of the key to sign with inside the keystore.

  :Default: ``androiddebugkey``, together with the default keystore

.. variable:: CPACK_APK_KEY_PASSWORD

  Password of the key.  A PKCS12 keystore, which is what ``keytool``
  creates by default since JDK 9, protects its keys with the keystore
  password and cannot hold a key with a password of its own.  Leave this
  unset for such a keystore.

  :Default: :variable:`CPACK_APK_KEYSTORE_PASSWORD`

.. variable:: CPACK_APK_ZIPALIGN_OPTIONS

  Options passed to ``zipalign`` ahead of the input and output files.
  The default aligns uncompressed native libraries to 16 KB memory pages,
  which devices with such pages require to load them straight from the
  APK.  Build tools older than version 35 only support 4 KB pages.

  :Default: ``-P;16;-f;4``, or ``-p;-f;4`` with build tools older than 35

.. variable:: CPACK_APK_AAPT2_EXECUTABLE
              CPACK_APK_D8_EXECUTABLE
              CPACK_APK_ZIPALIGN_EXECUTABLE
              CPACK_APK_APKSIGNER_EXECUTABLE
              CPACK_APK_JAR_EXECUTABLE
              CPACK_APK_KEYTOOL_EXECUTABLE

  Paths to the individual tools, overriding the ones that would be found
  in the SDK build tools directory and in the JDK.

  :Default: Detected

.. variable:: CPACK_APK_AAPT2_LINK_EXTRA_OPTIONS
              CPACK_APK_JAVAC_EXTRA_OPTIONS
              CPACK_APK_D8_EXTRA_OPTIONS
              CPACK_APK_APKSIGNER_EXTRA_OPTIONS

  Additional options to pass to the respective tool.  To compile the Java
  sources for another Java release, set :variable:`CPACK_APK_JAVA_VERSION`
  or pass ``javac`` the ``-source`` and ``-target`` options rather than
  ``--release``, which makes it compile against the Java library of the
  JDK instead of the one of the Android platform.  For a target release
  older than 9, ``android.jar`` is used as the boot class path instead of
  a system image.

  ``d8`` is passed ``--release``, which leaves the debugging information
  out of the Dalvik bytecode, unless ``--debug`` is given here.

  :Default: Unset

.. variable:: CPACK_APK_DEBUG

  Boolean enabling verbose output of every tool invocation and its
  result.

  :Default: ``FALSE``
