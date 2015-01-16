windows-utf-8
-------------

* On Windows, CMake learned to support international characters.
  This allows use of characters from multiple (spoken) languages
  in CMake code, paths to source files, configured files such as
  ``.h.in`` files, and other files read and written by CMake.
  Because CMake interoperates with many other tools, there may
  still be some limitations when using certain international
  characters.

  Files written in the :manual:`cmake-language(7)`, such as
  ``CMakeLists.txt`` or ``*.cmake`` files, are expected to be
  encoded as UTF-8.  If files are already ASCII, they will be
  compatible.  If files were in a different encoding, including
  Latin 1, they will need to be converted.

  The Visual Studio generators now write solution and project
  files in UTF-8 instead of Windows-1252.  Windows-1252 supported
  Latin 1 languages such as those found in North and South America
  and Western Europe.  With UTF-8, additional languages are now
  supported.
