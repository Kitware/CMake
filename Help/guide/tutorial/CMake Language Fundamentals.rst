Step 2: CMake Language Fundamentals
===================================

In the previous step we rushed through and handwaved several aspects of the
CMake language which is used within ``CMakeLists.txt`` in order to get useful,
building programs as soon as possible. However, in the wild we encounter
a great deal more complexity than simply describing lists of source and
header files.

To deal with this complexity CMake provides a Turing-complete domain-specific
language for describing the process of building software. Understanding the
fundamentals of this language will be necessary as we write more complex
CMLs and other CMake files. The language is formally known as
":manual:`CMake Language <cmake-language(7)>`", or more colloquially as CMakeLang.

.. note::
  The CMake Language is not well suited to describing things which are not
  related to building software. While it has some features for general purpose
  use, developers should use caution when solving problems not directly related
  to their build in CMake Language.

  Oftentimes the correct answer is to write a tool in a general purpose
  programming language which solves the problem, and teach CMake how to invoke
  that tool as part of the build process. Code generation, cryptographic
  signature utilities, and even ray-tracers have been written in CMake Language,
  but this is not a recommended practice.

Because we want to fully explore the language features, this step is an
exception to the tutorial sequencing. It neither builds on ``Step1``, nor is the
starting point for ``Step3``. This will be a sandbox to explore language
features without building any software. We'll pick back up with the Tutorial
program in ``Step3``.

.. note::
  This tutorial endeavors to demonstrate best practices and solutions to real
  problems. However, for this one step we're going to be re-implementing some
  built-in CMake functions. In "real life", do not write your own
  :command:`list(APPEND)`.

Background
^^^^^^^^^^

The only fundamental types in CMakeLang are strings and lists. Every object in
CMake is a string, and lists are themselves strings which contain semicolons
as separators. Any command which appears to operate on something other than a
string, whether they be booleans, numbers, JSON objects, or otherwise, is in
fact consuming a string, doing some internal conversion logic (in a language
other than CMakeLang), and then converting back to a string for any potential
output.

We can create a variable, which is to say a name for a string, using the
:command:`set` command.

.. code-block:: cmake

  set(var "World!")

A variable's value can be accessed using brace expansion, for example if we want
to use the :command:`message` command to print the string named by ``var``.

.. code-block:: cmake

  set(var "World!")
  message("Hello ${var}")

.. code-block:: console

  $ cmake -P CMakeLists.txt
  Hello World!

.. note::
  :option:`cmake -P` is called "script mode", it informs CMake this file is not
  intended to have a :command:`project` command. We're not building any
  software, instead using CMake only as a command interpreter.

Because CMakeLang has only strings, conditionals are entirely by convention of
which strings are considered true and which are considered false. These are
*supposed* to be intuitive, "True", "On", "Yes", and (strings representing)
non-zero numbers are truthy, while "False" "Off", "No", "0", "Ignore",
"NotFound", and the empty string are all considered false.

However, some of the rules are more complex than that, so taking some time
to consult the :command:`if` documentation on expressions is worthwhile. It's
recommended to stick to a single pair for a given context, such as
"True"/"False" or "On"/"Off".

As mentioned, lists are strings containing semicolons. The :command:`list`
command is useful for manipulating these, and many structures within CMake
expect to operate with this convention. As an example, we can use the
:command:`foreach` command to iterate over a list.

.. code-block:: cmake

  set(stooges "Moe;Larry")
  list(APPEND stooges "Curly")

  message("Stooges contains: ${stooges}")

  foreach(stooge IN LISTS stooges)
    message("Hello, ${stooge}")
  endforeach()

.. code-block:: console

  $ cmake -P CMakeLists.txt
  Stooges contains: Moe;Larry;Curly
  Hello, Moe
  Hello, Larry
  Hello, Curly

Exercise 1 - Macros, Functions, and Lists
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

CMake allows us to craft our own functions and macros. This can be very helpful
when constructing lots of similar targets, like tests, for which we will want
to call similar sets of commands over and over again. We do so with
:command:`function` and :command:`macro`.

.. code-block:: cmake

  macro(MyMacro MacroArgument)
    message("${MacroArgument}\n\t\tFrom Macro")
  endmacro()

  function(MyFunc FuncArgument)
    MyMacro("${FuncArgument}\n\tFrom Function")
  endfunction()

  MyFunc("From TopLevel")

.. code-block:: console

  $ cmake -P CMakeLists.txt
  From TopLevel
        From Function
                From Macro

Like with many languages, the difference between functions and macros is one
of scope. In CMakeLang, both :command:`function` and :command:`macro` can "see"
all the variables created in all the frames above them. However, a
:command:`macro` acts semantically like a text replacement, similar to C/C++
macros, so any side effects the macro creates are visible in their calling
context. If we create or change a variable in a macro, the caller will see the
change.

:command:`function` creates its own variable scope, so side effects are not
visible to the caller. In order to propagate changes to the parent which called
the function, we must use ``set(<var> <value> PARENT_SCOPE)``, which works the
same as :command:`set` but for variables belonging to the caller's context.

.. note::
  In CMake 3.25, the :command:`return(PROPAGATE)` option was added, which
  works the same as :command:`set(PARENT_SCOPE)` but provides slightly better
  ergonomics.

While not necessary for this exercise, it bears mentioning that :command:`macro`
and :command:`function` both support variadic arguments via the ``ARGV``
variable, a list containing all arguments passed to the command, and the
``ARGN`` variable, containing all arguments past the last expected argument.

We're not going to build any targets in this exercise, so instead we'll
construct our own version of :command:`list(APPEND)`, which adds a value to a
list.

Goal
----

Implement a macro and a function which append a value to a list, without using
the :command:`list(APPEND)` command.

The desired usage of these commands is as follows:

.. code-block:: cmake

  set(Letters "Alpha;Beta")
  MacroAppend(Letters "Gamma")
  message("Letters contains: ${Letters}")

.. code-block:: console

  $ cmake -P Exercise1.cmake
  Letters contains: Alpha;Beta;Gamma

.. note::
  The extension for these exercises is ``.cmake``, that's the standard extension
  for CMakeLang files when not contained in a ``CMakeLists.txt``

Helpful Resources
-----------------

* :command:`macro`
* :command:`function`
* :command:`set`
* :command:`if`

Files to Edit
-------------

* ``Exercise1.cmake``

Getting Started
----------------

The source code for ``Exercise1.cmake`` is provided in the
``Help/guide/tutorial/Step2`` directory. It contains tests to verify the
append behavior described above.

.. note::
  You're not expected to handle the case of an empty or undefined list to
  append to. However, as a bonus, the case is tested if you want to try out
  your understanding of CMakeLang conditionals.

Complete ``TODO 1`` and ``TODO 2``.

Build and Run
-------------

We're going to use script mode to run these exercises. First navigate to the
``Help/guide/tutorial/Step2`` folder then you can run the code with:

.. code-block:: console

  cmake -P Exercise1.cmake

The script will report if the commands were implemented correctly.

Solution
--------

This problem relies on an understanding of the mechanisms of CMake variables.
CMake variables are names for strings; or put another way, a CMake variable
is itself a string which can brace expand into a different string.

This leads to a common pattern in CMake code where functions and macros aren't
passed values, but rather, they are passed the names of variables which contain
those values. Thus ``ListVar`` does not contain the *value* of the list we need
to append to, it contains the *name* of a list, which contains the value we
need to append to.

When expanding the variable with ``${ListVar}``, we will get the name of the
list. If we expand that name with ``${${ListVar}}``, we will get the values
the list contains.

To implement ``MacroAppend``, we need only combine this understanding of
``ListVar`` with our knowledge of the :command:`set` command.

.. raw:: html

  <details><summary>TODO 1: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 1: Exercise1.cmake
  :name: Exercise1.cmake-MacroAppend

  macro(MacroAppend ListVar Value)
    set(${ListVar} "${${ListVar}};${Value}")
  endmacro()

.. raw:: html

  </details>

We don't need to worry about scope here, because a macro operates in the same
scope as its parent.

``FuncAppend`` is almost identical, in fact it could be implemented in the
same one liner but with an added ``PARENT_SCOPE``, but the instructions ask
us to implement it in terms of ``MacroAppend``.

.. raw:: html

  <details><summary>TODO 2: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 2: Exercise1.cmake
  :name: Exercise1.cmake-FuncAppend

  function(FuncAppend ListVar Value)
    MacroAppend(${ListVar} ${Value})
    set(${ListVar} "${${ListVar}}" PARENT_SCOPE)
  endfunction()

.. raw:: html

  </details>

``MacroAppend`` transforms ``ListVar`` for us, but it won't propagate the result
to the parent scope. Because this is a function, we need to do so ourselves
with :command:`set(PARENT_SCOPE)`.

Exercise 2 - Conditionals and Loops
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The two most common flow control elements in any structured programming
language are conditionals and their close sibling loops. CMakeLang is no
different. As previously mentioned, the truthiness of a given CMake string is a
convention established by the :command:`if` command.

When given a string, :command:`if` will first check if it is one of the known
constant values previously discussed. If the string isn't one of those values
the command assumes it is a variable, and checks the brace-expanded contents of
that variable to determine the result of the conditional.

.. code-block:: cmake

  if(True)
    message("Constant Value: True")
  else()
    message("Constant Value: False")
  endif()

  if(ConditionalValue)
    message("Undefined Variable: True")
  else()
    message("Undefined Variable: False")
  endif()

  set(ConditionalValue True)

  if(ConditionalValue)
    message("Defined Variable: True")
  else()
    message("Defined Variable: False")
  endif()

.. code-block:: console

  $ cmake -P ConditionalValue.cmake
  Constant Value: True
  Undefined Variable: False
  Defined Variable: True

.. note::
    This is a good a time as any to discuss quoting in CMake. All objects in
    CMake are strings, thus the double quote, ``"``, is often unnecessary.
    CMake knows the object is a string, everything is a string.

    However, it is needed in some contexts. Strings containing whitespace require
    double quotes, else they are treated like lists; CMake will concatenate the
    elements together with semicolons. The reverse is also true, when
    brace-expanding lists it is necessary to do so inside quotes if we want to
    *preserve* the semicolons. Otherwise CMake will expand the list items into
    space-separate strings.

    A handful of commands, such as :command:`if`, recognize the difference
    between quoted and unquoted strings. :command:`if` will only check that the
    given string represents a variable when the string is unquoted.

Finally, :command:`if` provides several useful comparison modes such as
``STREQUAL`` for string matching, ``DEFINED`` for checking the existence of
a variable, and ``MATCHES`` for regular expression checks. It also supports the
typical logical operators, ``NOT``, ``AND``, and ``OR``.

In addition to conditionals CMake provides two loop structures,
:command:`while`, which follows the same rules as :command:`if` for checking a
loop variable, and the more useful :command:`foreach`, which iterates over lists
of strings and was demonstrated in the `Background`_ section.

For this exercise, we're going to use loops and conditionals to solve some
simple problems. We'll be using the aforementioned ``ARGN`` variable from
:command:`function` as the list to operate on.

Goal
----

Loop over a list, and return all the strings containing the string ``Foo``.

.. note::
  Those who read the command documentation will be aware that this is
  :command:`list(FILTER)`, resist the temptation to use it.

Helpful Resources
-----------------

* :command:`function`
* :command:`foreach`
* :command:`if`
* :command:`list`

Files to Edit
-------------

* ``Exercise2.cmake``

Getting Started
----------------

The source code for ``Exercise2.cmake`` is provided in the ``Help/guide/tutorial/Step2``
directory. It contains tests to verify the append behavior described above.

.. note::
  You should use the :command:`list(APPEND)` command this time to collect your
  final result into a list. The input can be consumed from the ``ARGN`` variable
  of the provided function.

Complete ``TODO 3``.

Build and Run
-------------

Navigate to the ``Help/guide/tutorial/Step2`` folder then you can run the code with:

.. code-block:: console

  cmake -P Exercise2.cmake

The script will report if the ``FilterFoo`` function was implemented correctly.

Solution
--------

We need to do three things, loop over the ``ARGN`` list, check if a given
item in that list matches ``"Foo"``, and if so append it to the ``OutVar``
list.

While there are a couple ways we could invoke :command:`foreach`, the
recommended way is to allow the command to do the variable expansion for us
via ``IN LISTS`` to access the ``ARGN`` list items.

The :command:`if` comparison we need is ``MATCHES`` which will check if
``"FOO"`` exists in the item. All that remains is to append the item to the
``OutVar`` list.  The trickiest part is remembering that ``OutVar`` *names* a
list, it is not the list itself, so we need to access it via ``${OutVar}``.

.. raw:: html

  <details><summary>TODO 3: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 3: Exercise2.cmake
  :name: Exercise2.cmake-FilterFoo

  function(FilterFoo OutVar)

    foreach(item IN LISTS ARGN)
      if(item MATCHES Foo)
        list(APPEND ${OutVar} ${item})
      endif()
    endforeach()

    set(${OutVar} ${${OutVar}} PARENT_SCOPE)
  endfunction()

.. raw:: html

  </details>

Exercise 3 - Organizing with Include
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

We have already discussed how to incorporate subdirectories containing their
own CMLs with :command:`add_subdirectory`. In later steps we will explore
the various way CMake code can be packaged and shared across projects.

However for small CMake functions and utilities, it is often beneficial for them
to live in their own ``.cmake`` files outside the project CMLs and separate
from the rest of the build system. This allows for separation of concerns,
removing the project-specific elements from the utilities we are using to
describe them.

To incorporate these separate ``.cmake`` files into our project, we use the
:command:`include` command. This command immediately begins interpreting the
contents of the :command:`include`'d file in the scope of the parent CML. It
is as if the entire file were being called as a macro.

Traditionally, these kinds of ``.cmake`` files live in a folder named "cmake"
inside the project root. For this exercise, we'll use the ``Step2`` folder instead.

Goal
----

Use the functions from Exercises 1 and 2 to build and filter our own list of items.

Helpful Resources
-----------------

* :command:`include`

Files to Edit
-------------

* ``Exercise3.cmake``

Getting Started
----------------

The source code for ``Exercise3.cmake`` is provided in the ``Help/guide/tutorial/Step2``
directory. It contains tests to verify the correct usage of our functions
from the previous two exercises.

.. note::
  Actually it reuses tests from Exercise2.cmake, reusable code is good for
  everyone.

Complete ``TODO 4`` through ``TODO 7``.

Build and Run
-------------

Navigate to the ``Help/guide/tutorial/Step2`` folder then you can run the code with:

.. code-block:: console

  cmake -P Exercise3.cmake

The script will report if the functions were invoked and composed correctly.

Solution
--------

The :command:`include` command will interpret the included file completely,
including the tests from the first two exercises. We don't want to run these
tests again. Thanks to some forethought, these files check a variable called
``SKIP_TESTS`` prior to running their tests, setting this to ``True`` will
get us the behavior we want.

.. raw:: html

  <details><summary>TODO 4: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 4: Exercise3.cmake
  :name: Exercise3.cmake-SKIP_TESTS

  set(SKIP_TESTS True)

.. raw:: html

  </details>

Now we're ready to :command:`include` the previous exercises to grab their
functions.

.. raw:: html

  <details><summary>TODO 5: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 5: Exercise3.cmake
  :name: Exercise3.cmake-include

  include(Exercise1.cmake)
  include(Exercise2.cmake)

.. raw:: html

  </details>

Now that ``FuncAppend`` is available to us, we can use it to append new elements
to the ``InList``.

.. raw:: html

  <details><summary>TODO 6: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 6: Exercise3.cmake
  :name: Exercise3.cmake-FuncAppend

  FuncAppend(InList FooBaz)
  FuncAppend(InList QuxBaz)

.. raw:: html

  </details>

Finally, we can use ``FilterFoo`` to filter the full list. The tricky part to
remember here is that our ``FilterFoo`` wants to operate on list values via
``ARGN``, so we need to expand the ``InList`` when we call ``FilterFoo``.

.. raw:: html

  <details><summary>TODO 7: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 7: Exercise3.cmake
  :name: Exercise3.cmake-FilterFoo

  FilterFoo(OutList ${InList})

.. raw:: html

  </details>
