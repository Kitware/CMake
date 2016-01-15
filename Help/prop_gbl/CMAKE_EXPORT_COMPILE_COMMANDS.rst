CMAKE_EXPORT_COMPILE_COMMANDS
-----------------------------

Enable/Disable output of compile commands during generation.

If enbaled, generate a ``compile_commands.json`` file containing the exact
calls for all translation units of the project in machine-readable form.

The format of the JSON file looks like:

.. code-block:: cmake

  [
    {
      "directory": "/home/user/development/project",
      "command": "/usr/bin/c++ -DHAVE_CONFIG_H -DDEBUG -Wall -g -O0 -fPIC -I../foo/include -std=gnu++11 -MMD -MT ../foo/CMakeFiles/foo.dir/foo.cc.o -MF ../foo/CMakeFiles/foo.dir/foo.cc.o.d -o ../foo/CMakeFiles/foo.dir/foo.cc.o -c ../foo/foo.cc",
      "file": "../foo/foo.cc"
    },

    ...

    {
      "directory": "/home/user/development/project",
      "command": "/usr/bin/c++ -DHAVE_CONFIG_H -DDEBUG -Wall -g -O0 -fPIC -I../foo/include -std=gnu++11 -MMD -MT ../foo/CMakeFiles/foo.dir/bar.cc.o -MF ../foo/CMakeFiles/foo.dir/bar.cc.o.d -o ../foo/CMakeFiles/foo.dir/bar.cc.o -c ../foo/bar.cc",
      "file": "../foo/bar.cc"
    }
  ]

Note that this global properly currently is only implemented for the :generator:`Unix Makefiles` and :generator:`Ninja` generators.
