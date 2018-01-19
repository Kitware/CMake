cache-newline
-------------

* Variables containing newlines in their values now get truncated before the
  newline when they are written to the cache file. In addition, a warning
  comment is written to the cache file, and a warning message is displayed to
  the user on the console.
