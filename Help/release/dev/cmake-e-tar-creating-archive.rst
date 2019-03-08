cmake-e-tar-creating-archive
----------------------------

* The :manual:`cmake(1)` ``-E tar`` tool now continues adding files to an
  archive, even if some of the files aren't readable. This behavior is more
  consistent with the classic ``tar`` tool.
