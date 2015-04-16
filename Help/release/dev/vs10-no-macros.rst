vs10-no-macros
--------------

* The :generator:`Visual Studio 10 2010` generator no longer checks
  for running VS IDEs with the project open or asks them to reload.
  This was originally done for VS 10 because it had been done for
  VS 7 through 9 to avoid prompting for every project in a solution.
  Since VS >= 10 allow the whole solution to reload at once they
  do not need CMake to help them.
