LINK_WHAT_YOU_USE
-----------------

.. versionadded:: 3.7

This is a boolean option that, when set to ``TRUE``, will automatically run
contents of variable :variable:`CMAKE_LINK_WHAT_YOU_USE_CHECK` on the target
after it is linked. In addition, the linker flag specified by variable
:variable:`CMAKE_<LANG>_LINK_WHAT_YOU_USE_FLAG`  will be passed to the target
with the link command so that all libraries specified on the command line will
be linked into the target. This will result in the link producing a list of
libraries that provide no symbols used by this target but are being linked to
it.

.. note::

  For now, it is only supported for ``ELF`` platforms and is only applicable to
  executable and shared or module library targets. This property will be
  ignored for any other targets and configurations.

This property is initialized by the value of
the :variable:`CMAKE_LINK_WHAT_YOU_USE` variable if it is set
when a target is created.
