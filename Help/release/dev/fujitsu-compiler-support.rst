fujitsu-compiler-support
------------------------

* Addition of the ``Fujitsu`` compiler ID operating in traditional ``Trad``
  mode and ``FujitsuClang`` operating in ``Clang`` mode.
* The :module:`FindOpenMP` module learned to support ``Fujitsu`` and
  ``FujitsuClang``.
* The :module:`FindMPI` module learned to support ``Fujitsu`` and
  ``FujitsuClang`` in both host and cross compiling modes.
* The :module:`FindBLAS` and :module:`FindLAPACK` modules learned to support
  the serial ``Fujitsu SSL2`` and parallel ``Fujitsu SSL2BLAMP`` libraries.
