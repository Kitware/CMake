STRING:RANDOM
-------------

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:RANDOM[,(LENGTH:length|ALPHABET:alphabet|RANDOM_SEED:seed)]...>

.. versionadded:: 4.3

Produce a random string of ASCII characters. The possible options are:

``LENGTH:length``
  Define the length of the string. The default length is 5 characters.

``ALPHABET:alphabet``
  Define the characters used for the generation. The alphabet is always
  interpreted as holding ASCII characters. The default alphabet is all
  numbers and upper and lower case letters.

``RANDOM_SEED:seed``
  Specify an integer which will be used to seed the random number generator.
