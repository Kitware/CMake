FindOpenSSL-separate-libs
-------------------------

* The :module:`FindOpenSSL` module now reports ``crypto`` and ``ssl``
  libraries separately in ``OPENSSL_CRYPTO_LIBRARY`` and
  ``OPENSSL_SSL_LIBRARY``, respectively, to allow applications to
  link to one without the other.
