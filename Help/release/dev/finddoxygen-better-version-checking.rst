finddoxygen-better-version-checking
-----------------------------------

* The :module:`FindDoxygen` module now evaluates as many candidate
  Doxygen installs as are necessary to satisfy version constraints,
  with the package considered to be not found if none are available.

* The :module:`FindDoxygen` module now handles version ranges.

* The :module:`FindDoxygen` module now ignores non-semantic portions
  of the output from Doxygen's `--version` option.
