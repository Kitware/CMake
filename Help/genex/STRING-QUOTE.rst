STRING:QUOTE
------------

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:QUOTE,REGEX,string>

.. versionadded:: 4.3

Escape the specified elements of the ``string``. The possible options are:

``REGEX``
  Escape all characters that have special meaning in a regular expressions,
  such that the ``string`` can be used as part of a regular expression to
  match the input literally.
