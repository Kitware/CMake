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

  The specified items will be added to the ``Embed App Extensions`` build
  phase, with ``Destination`` set to ``PlugIns and Foundation Extensions``
  They must be CMake target names.

``EXTENSIONKIT_EXTENSIONS``
  .. versionadded:: 3.26

  The specified items will be added to the ``Embed App Extensions`` build
  phase, with ``Destination`` set to ``ExtensionKit Extensions``
  They must be CMake target names, and should likely have the
  ``XCODE_PRODUCT_TYPE`` target property set to
  ``com.apple.product-type.extensionkit-extension``
  as well as the  ``XCODE_EXPLICIT_FILE_TYPE`` to
  ``wrapper.extensionkit-extension``

``PLUGINS``
  .. versionadded:: 3.23

  The specified items will be added to the ``Embed PlugIns`` build phase.
  They must be CMake target names.

``RESOURCES``
  .. versionadded:: 3.28

  The specified items will be added to the ``Embed Resources`` build phase.
  They must be CMake target names.

See also :prop_tgt:`XCODE_EMBED_<type>_PATH`,
:prop_tgt:`XCODE_EMBED_<type>_REMOVE_HEADERS_ON_COPY` and
:prop_tgt:`XCODE_EMBED_<type>_CODE_SIGN_ON_COPY`.
