get-runtime-dependencies
------------------------

* The :command:`file` command learned a new sub-command,
  ``GET_RUNTIME_DEPENDENCIES``, which allows you to recursively get the list of
  libraries linked by an executable or library. This sub-command is intended as
  a replacement for :module:`GetPrerequisites`.
* The :module:`GetPrerequisites` module has been deprecated, as it has been
  superceded by :command:`file(GET_RUNTIME_DEPENDENCIES)`.
