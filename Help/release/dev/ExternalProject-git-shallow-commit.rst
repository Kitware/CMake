ExternalProject-git-shallow-commit
----------------------------------

* The :module:`ExternalProject` module's ``GIT_SHALLOW`` option, also used
  by the :module:`FetchContent` module, now accepts a full commit hash in
  ``GIT_TAG``.  A commit that is neither a branch tip nor a tag, which a
  shallow clone does not contain, is fetched by itself.
