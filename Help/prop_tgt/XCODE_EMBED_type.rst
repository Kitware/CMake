XCODE_EMBED_<type>
------------------

.. versionadded:: 3.20

Tell the :generator:`Xcode` generator to embed the specified list of items into
the target bundle.  ``<type>`` specifies the embed build phase to use.

Currently, the only supported value for ``<type>`` is ``FRAMEWORKS``.
The specified items will be added to the ``Embed Frameworks`` build phase.
The items can be CMake target names or paths to frameworks or libraries.
See also :prop_tgt:`XCODE_EMBED_<type>_PATH`,
:prop_tgt:`XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY` and
:prop_tgt:`XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY`.
