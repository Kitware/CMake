curl_netrc_options
------------------

* The :command:`file(DOWNLOAD)` and :command:`file(UPLOAD)` commands
  gained ``NETRC`` and ``NETRC_FILE`` options to specify use of a
  ``.netrc`` file.

* The :module:`ExternalProject` module gained ``NETRC`` and ``NETRC_FILE``
  options to specify use of a ``.netrc`` file.

* The :variable:`CMAKE_NETRC` and :variable:`CMAKE_NETRC_FILE` variables
  were added to specify use of a ``.netrc`` file by the
  :command:`file(DOWNLOAD)` and :command:`file(UPLOAD)` commands and
  the :module:`ExternalProject` module.
