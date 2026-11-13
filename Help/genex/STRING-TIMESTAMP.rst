STRING:TIMESTAMP
----------------

.. |genex-family| replace::
   :ref:`genex-family-string-generating-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:TIMESTAMP[,(UTC|format)]...>

.. versionadded:: 4.3

Produce a string representation of the current date and/or time.

If the generator expression is unable to obtain a timestamp, the result will be
the empty string ``""``.

The optional ``UTC`` flag requests the current date/time representation to be in
Coordinated Universal Time (UTC) rather than local time.

If the ``SOURCE_DATE_EPOCH`` environment variable is set, its value will be used
instead of the current time. See
https://reproducible-builds.org/specs/source-date-epoch/ for details.

The optional ``<format>`` may contain the following format specifiers:

``%%``
  A literal percent sign (%).

``%d``
  The day of the current month (01-31).

``%H``
  The hour on a 24-hour clock (00-23).

``%I``
  The hour on a 12-hour clock (01-12).

``%j``
  The day of the current year (001-366).

``%m``
  The month of the current year (01-12).

``%b``
  Abbreviated month name (e.g. Oct).

``%B``
  Full month name (e.g. October).

``%M``
  The minute of the current hour (00-59).

``%s``
  Seconds since midnight (UTC) 1-Jan-1970 (UNIX time).

``%S``
  The second of the current minute.  60 represents a leap second. (00-60)

``%f``
  The microsecond of the current second (000000-999999).

``%U``
  The week number of the current year (00-53).

``%V``
  The ISO 8601 week number of the current year (01-53).

``%w``
  The day of the current week. 0 is Sunday. (0-6)

``%a``
  Abbreviated weekday name (e.g. Fri).

``%A``
  Full weekday name (e.g. Friday).

``%y``
  The last two digits of the current year (00-99).

``%Y``
  The current year.

``%z``
  The offset of the time zone from UTC, in hours and minutes,
  with format ``+hhmm`` or ``-hhmm``.

``%Z``
  The time zone name.

Unknown format specifiers will be ignored and copied to the output as-is.

If no explicit ``format`` is given, it will default to:

* ``%Y-%m-%dT%H:%M:%S`` for local time.
* ``%Y-%m-%dT%H:%M:%SZ`` for UTC.
