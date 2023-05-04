use-linker-depfile
------------------

* GNU (and GNU-compatible) linkers gained support for a ``--dependency-file``
  flag in GNU Binutils 2.35 and LLVM's LLD 12.0.0. The
  :ref:`Makefile <Makefile Generators>` and :ref:`Ninja <Ninja Generators>`
  generators will now add these flags so that files read by the linker will
  cause a relink if they change (typically modified timestamps).

  This feature can be controlled by the variable
  :variable:`CMAKE_LINK_DEPENDS_USE_LINKER`.
