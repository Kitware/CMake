file-generate-relative-paths
----------------------------

* The :command:`file(GENERATE)` command now interprets relative paths
  given to its ``OUTPUT`` and ``INPUT`` arguments with respect to the
  caller's current binary and source directories, respectively.
  See policy :policy:`CMP0070`.
