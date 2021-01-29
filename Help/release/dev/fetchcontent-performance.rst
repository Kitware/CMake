fetchcontent-performance
------------------------

* The implementation of the :module:`ExternalProject` module was
  significantly refactored.  The patch step gained support for
  using the terminal with a new ``USES_TERMINAL_PATCH`` keyword
  as a by-product of that work.
* The :module:`FetchContent` module no longer creates a separate
  sub-build to implement the content population.  It now invokes
  the step scripts directly from within the main project's
  configure stage.  This significantly speeds up the configure
  phase when the required content is already populated and
  up-to-date.
