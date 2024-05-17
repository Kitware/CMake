fetchcontent-direct
-------------------

* :module:`FetchContent` now prefers to populate content directly rather
  than using a separate sub-build. This may significantly improve configure
  times on some systems (Windows especially, but also on macOS when using
  the Xcode generator). Policy :policy:`CMP0168` provides backward
  compatibility for those projects that still rely on using a sub-build for
  content population.
