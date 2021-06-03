XCODE_EMBED_<type>
------------------

.. versionadded:: 3.20

Tell the :generator:`Xcode` generator to embed the specified list of items into
the target bundle.  ``<type>`` specifies the embed build phase to use.
See the Xcode documentation for the base location of each ``<type>``.

The supported values for ``<type>`` are:

``FRAMEWORKS``
  The specified items will be added to the ``Embed Frameworks`` build phase.
  The items can be CMake target names or paths to frameworks or libraries.

``APP_EXTENSIONS``
  .. versionadded:: 3.21

  The specified items will be added to the ``Embed App Extensions`` build phase.
  They must be CMake target names.

See also :prop_tgt:`XCODE_EMBED_<type>_PATH`,
:prop_tgt:`XCODE_EMBED_<type>_REMOVE_HEADERS_ON_COPY` and
:prop_tgt:`XCODE_EMBED_<type>_CODE_SIGN_ON_COPY`.
