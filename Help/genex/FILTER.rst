FILTER
------

Syntax
^^^^^^

.. code-block:: cmake

  $<FILTER:list,INCLUDE|EXCLUDE,regex>

.. versionadded:: 3.15

Includes or removes items from ``list`` that match the regular expression
``regex``.  The result is the same as
:cref:`$\<LIST:FILTER,list,INCLUDE|EXCLUDE,regex\> <GenEx LIST-FILTER>`.
