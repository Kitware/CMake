FetchContent
------------

* A new :module:`FetchContent` module was added which supports populating
  content at configure time using any of the download/update methods
  supported by :command:`ExternalProject_Add`.  This allows the content
  to be used immediately during the configure stage, such as with
  :command:`add_subdirectory`, etc.  Hierarchical project structures are
  well supported, allowing parent projects to override the content details
  of child projects and ensuring content is not populated multiple times
  throughout the whole project tree.
