CMAKE_CXX_COMPILER_IMPORT_STD
-----------------------------

.. versionadded:: 4.3

A list of C++ standard levels for which ``import std`` support exists for the
current C++ toolchain.  Support for C++\<NN\> may be detected using a
``<NN> IN_LIST CMAKE_CXX_COMPILER_IMPORT_STD`` predicate with the
:command:`if` command.
