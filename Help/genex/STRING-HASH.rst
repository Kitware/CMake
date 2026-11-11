STRING:HASH
-----------

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:HASH,string,ALGORITHM:algorithm>

.. versionadded:: 4.3

Compute a cryptographic hash of the ``string``. The supported algorithm names,
as specified by the ``ALGORITHM:`` option are:

``MD5``
  Message-Digest Algorithm 5, RFC 1321.
``SHA1``
  US Secure Hash Algorithm 1, RFC 3174.
``SHA224``
  US Secure Hash Algorithms, RFC 4634.
``SHA256``
  US Secure Hash Algorithms, RFC 4634.
``SHA384``
  US Secure Hash Algorithms, RFC 4634.
``SHA512``
  US Secure Hash Algorithms, RFC 4634.
``SHA3_224``
  Keccak SHA-3.
``SHA3_256``
  Keccak SHA-3.
``SHA3_384``
  Keccak SHA-3.
``SHA3_512``
  Keccak SHA-3.
