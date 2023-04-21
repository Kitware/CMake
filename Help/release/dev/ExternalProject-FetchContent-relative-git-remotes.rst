ExternalProject-FetchContent-Relative-git-remotes
-------------------------------------------------

* The :module:`ExternalProject` and :module:`FetchContent` modules
  now resolve relative `GIT_REPOSITORY` paths as relative to the
  parent project's remote, not as a relative local file system path.
  See :policy:`CMP0150`.
