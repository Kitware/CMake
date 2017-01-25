cpack-ifw-options
-----------------

* The :module:`CPackIFW` module :command:`cpack_ifw_configure_component` and
  :command:`cpack_ifw_configure_component_group` commands gained a new
  ``DEFAULT``, ``VIRTUAL``, ``FORCED_INSTALLATION``, ``REQUIRES_ADMIN_RIGHTS``,
  ``DISPLAY_NAME``, ``UPDATE_TEXT``, ``DESCRIPTION``, ``RELEASE_DATE``,
  ``AUTO_DEPEND_ON`` and ``TRANSLATIONS`` options to more specific
  configuration.

* The :module:`CPackIFW` module :command:`cpack_ifw_configure_component`
  command gained a new ``DEPENDENCIES`` alias for ``DEPENDS`` option.

* The :module:`CPackIFW` module :command:`cpack_ifw_configure_component_group`
  command gained a new ``DEPENDS`` option. The ``DEPENDENCIES`` alias also
  added.

* The :module:`CPackIFW` module :command:`cpack_ifw_configure_component` and
  :command:`cpack_ifw_configure_component_group` commands ``PRIORITY``
  option now is deprecated and will be removed in a future version of CMake.
  Please use new ``SORTING_PRIORITY`` option instead.
