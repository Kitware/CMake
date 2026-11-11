STRING:UUID
-----------

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:UUID,NAMESPACE:namespace,TYPE:(MD5|SHA1)[,NAME:name][,CASE:(LOWER|UPPER)]>

.. versionadded:: 4.3

Create a universally unique identifier (aka GUID) as per RFC4122. A UUID has the
format ``xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`` where each ``x`` represents an
hexadecimal character.

The UUID is based on the hash of the combined values of:

``NAMESPACE:namespace``
  ``namespace`` which has to be a valid UUID.

``NAME:name``
  ``name`` is an arbitrary string.

``TYPE:``
  The hash algorithm can be either:

  ``MD5``
    Version 3 UUID.

  ``SHA1``
    Version 5 UUID.

``CASE:``
  Specify the case of the hexadecimal characters.

  ``LOWER``
    Hexadecimal characters are all of lowercase. This is the default.

  ``UPPER``
    Hexadecimal characters are all of uppercase.
