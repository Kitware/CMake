CMake Source Code Guide
***********************

The following is a guide to the CMake source code for developers.
See documentation on `CMake Development`_ for more information.

.. _`CMake Development`: README.rst

C++ Code Style
==============

We use `clang-format`_ version **15** to define our style for C++ code in
the CMake source tree.  See the `.clang-format`_ configuration file for our
style settings.  Use the `Utilities/Scripts/clang-format.bash`_ script to
format source code.  It automatically runs ``clang-format`` on the set of
source files for which we enforce style.  The script also has options to
format only a subset of files, such as those that are locally modified.

.. _`clang-format`: https://clang.llvm.org/docs/ClangFormat.html
.. _`.clang-format`: ../../.clang-format
.. _`Utilities/Scripts/clang-format.bash`: ../../Utilities/Scripts/clang-format.bash

C++ Subset Permitted
====================

CMake requires compiling as C++11 in order to support building on older
toolchains.  However, to facilitate development, some standard library
features from more recent C++ standards are supported through a compatibility
layer.  These features are defined under the namespace ``cm`` and headers
are accessible under the ``cm/`` directory.  The headers under ``cm/`` can
be used in place of the standard ones when extended features are needed.
For example ``<cm/memory>`` can be used in place of ``<memory>``.

Available features are:

* From ``C++14``:

  * ``<cm/array>``:
    ``cm::cbegin``, ``cm::cend``, ``cm::rbegin``, ``cm::rend``,
    ``cm::crbegin``, ``cm::crend``

  * ``<cm/deque>``:
    ``cm::cbegin``, ``cm::cend``, ``cm::rbegin``, ``cm::rend``,
    ``cm::crbegin``, ``cm::crend``

  * ``<cm/forward_list>``:
    ``cm::cbegin``, ``cm::cend``, ``cm::rbegin``, ``cm::rend``,
    ``cm::crbegin``, ``cm::crend``

  * ``<cm/iomanip>``:
    ``cm::quoted``

  * ``<cm/iterator>``:
    ``cm::make_reverse_iterator``, ``cm::cbegin``, ``cm::cend``,
    ``cm::rbegin``, ``cm::rend``, ``cm::crbegin``, ``cm::crend``

  * ``<cm/list>``:
    ``cm::cbegin``, ``cm::cend``, ``cm::rbegin``, ``cm::rend``,
    ``cm::crbegin``, ``cm::crend``

  * ``<cm/map>``:
    ``cm::cbegin``, ``cm::cend``, ``cm::rbegin``, ``cm::rend``,
    ``cm::crbegin``, ``cm::crend``

  * ``<cm/memory>``:
    ``cm::make_unique``

  * ``<cm/set>``:
    ``cm::cbegin``, ``cm::cend``, ``cm::rbegin``, ``cm::rend``,
    ``cm::crbegin``, ``cm::crend``

  * ``<cm/string>``:
    ``cm::cbegin``, ``cm::cend``, ``cm::rbegin``, ``cm::rend``,
    ``cm::crbegin``, ``cm::crend``

  * ``<cm/string_view>``:
    ``cm::cbegin``, ``cm::cend``, ``cm::rbegin``, ``cm::rend``,
    ``cm::crbegin``, ``cm::crend``

  * ``<cm/shared_mutex>``:
    ``cm::shared_lock``

  * ``<cm/type_traits>``:
    ``cm::enable_if_t``

  * ``<cm/unordered_map>``:
    ``cm::cbegin``, ``cm::cend``, ``cm::rbegin``, ``cm::rend``,
    ``cm::crbegin``, ``cm::crend``

  * ``<cm/unordered_set>``:
    ``cm::cbegin``, ``cm::cend``, ``cm::rbegin``, ``cm::rend``,
    ``cm::crbegin``, ``cm::crend``

  * ``<cm/vector>``:
    ``cm::cbegin``, ``cm::cend``, ``cm::rbegin``, ``cm::rend``,
    ``cm::crbegin``, ``cm::crend``

* From ``C++17``:

  * ``<cm/algorithm>``:
    ``cm::clamp``

  * ``<cm/array>``:
    ``cm::size``, ``cm::empty``, ``cm::data``

  * ``<cm/deque>``:
    ``cm::size``, ``cm::empty``, ``cm::data``

  * ``cm/filesystem>``:
    ``cm::filesystem::path``

  * ``<cm/forward_list>``:
    ``cm::size``, ``cm::empty``, ``cm::data``

  * ``<cm/iterator>``:
    ``cm::size``, ``cm::empty``, ``cm::data``

  * ``<cm/list>``:
    ``cm::size``, ``cm::empty``, ``cm::data``

  * ``<cm/map>``:
    ``cm::size``, ``cm::empty``, ``cm::data``

  * ``<cm/optional>``:
    ``cm::nullopt_t``, ``cm::nullopt``, ``cm::optional``,
    ``cm::make_optional``, ``cm::bad_optional_access``

  * ``<cm/set>``:
    ``cm::size``, ``cm::empty``, ``cm::data``

  * ``<cm/shared_mutex>``:
    ``cm::shared_mutex``

  * ``<cm/string>``:
    ``cm::size``, ``cm::empty``, ``cm::data``

  * ``<cm/string_view>``:
    ``cm::string_view``, ``cm::size``, ``cm::empty``, ``cm::data``

  * ``<cm/type_traits>``:
    ``cm::bool_constant``, ``cm::invoke_result_t``, ``cm::invoke_result``,
    ``cm::void_t``

  * ``<cm/unordered_map>``:
    ``cm::size``, ``cm::empty``, ``cm::data``

  * ``<cm/unordered_set>``:
    ``cm::size``, ``cm::empty``, ``cm::data``

  * ``<cm/utility>``:
    ``cm::in_place_t``, ``cm::in_place``

  * ``<cm/vector>``:
    ``cm::size``, ``cm::empty``, ``cm::data``

* From ``C++20``:

  * ``<cm/array>``:
    ``cm::ssize``

  * ``<cm/deque>``:
    ``cm::erase``, ``cm::erase_if``, ``cm::ssize``

  * ``<cm/forward_list>``:
    ``cm::ssize``

  * ``<cm/iterator>``:
    ``cm::ssize``

  * ``<cm/list>``:
    ``cm::erase``, ``cm::erase_if``, ``cm::ssize``

  * ``<cm/map>`` :
    ``cm::erase_if``, ``cm::ssize``

  * ``<cm/set>`` :
    ``cm::erase_if``, ``cm::ssize``

  * ``<cm/string_view>``:
    ``cm::ssize``

  * ``<cm/string>``:
    ``cm::erase``, ``cm::erase_if``, ``cm::ssize``

  * ``<cm/unordered_map>``:
    ``cm::erase_if``, ``cm::ssize``

  * ``<cm/unordered_set>``:
    ``cm::erase_if``, ``cm::ssize``

  * ``<cm/vector>``:
    ``cm::erase``, ``cm::erase_if``, ``cm::ssize``

Additionally, some useful non-standard extensions to the C++ standard library
are available in headers under the directory ``cmext/`` in namespace ``cm``.
These are:

* ``<cmext/algorithm>``:

  * ``cm::append``:
    Append elements to a sequential container.

  * ``cm::contains``:
    Checks if element or key is contained in container.

* ``<cmext/enum_set>``

  * ``cm::enum_set``:
    Container to manage set of elements from an ``enum class`` definition.

* ``<cmext/iterator>``:

  * ``cm::is_terator``:
    Checks if a type is an iterator type.

  * ``cm::is_input_iterator``:
    Checks if a type is an input iterator type.

  * ``cm::is_range``:
    Checks if a type is a range type: functions ``std::begin()`` and
    ``std::end()`` apply.

  * ``cm::is_input_range``:
    Checks if a type is an input range type: functions ``std::begin()`` and
    ``std::end()`` apply and return an input iterator.

* ``<cmext/memory>``:

  * ``cm::static_reference_cast``:
    Apply a ``static_cast`` to a smart pointer.

  * ``cm::dynamic_reference_cast``:
    Apply a ``dynamic_cast`` to a smart pointer.

* ``<cmext/type_traits>``:

  * ``cm::is_container``:
    Checks if a type is a container type.

  * ``cm::is_associative_container``:
    Checks if a type is an associative container type.

  * ``cm::is_unordered_associative_container``:
    Checks if a type is an unordered associative container type.

  * ``cm::is_sequence_container``:
    Checks if a type is a sequence container type.

  * ``cm::is_unique_ptr``:
    Checks if a type is a ``std::unique_ptr`` type.

CMake assumes the compiler supports ``#pragma once``. Use this for all
hand-written header files.

Dynamic Memory Management
=========================

To ensure efficient memory management, i.e. no memory leaks, it is required
to use smart pointers.  Any dynamic memory allocation must be handled by a
smart pointer such as ``std::unique_ptr`` or ``std::shared_ptr``.

It is allowed to pass raw pointers between objects to enable objects sharing.
A raw pointer **must** not be deleted. Only the object(s) owning the smart
pointer are allowed to delete dynamically allocated memory.

Third Parties
=============

To build CMake, some third parties are needed. Under ``Utilities``
directory, are versions of these third parties which can be used as an
alternate to the ones provided by the system.

To enable the selection of the third parties between the system and CMake ones,
in CMake sources, third parties headers must be prefixed by ``cm3p/``
(for example: ``<cm3p/json/reader.h>``). These wrappers are located under
``Utilities/cm3p`` directory.

Source Tree Layout
==================

The CMake source tree is organized as follows.

* ``Auxiliary/``:
  Shell and editor integration files.

* ``Help/``:
  Documentation.  See the `CMake Documentation Guide`_.

  * ``Help/dev/``:
    Developer documentation.

  * ``Help/release/dev/``:
    Release note snippets for development since last release.

* ``Licenses/``:
  License files for third-party libraries in binary distributions.

* ``Modules/``:
  CMake language modules installed with CMake.

* ``Packaging/``:
  Files used for packaging CMake itself for distribution.

* ``Source/``:
  Source code of CMake itself.

* ``Templates/``:
  Files distributed with CMake as implementation details for generators,
  packagers, etc.

* ``Tests/``:
  The test suite.  See `Tests/README.rst`_.

* ``Utilities/``:
  Scripts, third-party source code.

  * ``Utilities/std/cm``:
    Support files for various C++ standards.

  * ``Utilities/std/cmext``:
    Extensions to the C++ STL.

  * ``Utilities/cm3p``:
    Public headers for third parties needed to build CMake.

  * ``Utilities/Sphinx/``:
    Sphinx configuration to build CMake user documentation.

  * ``Utilities/Release/``:
    Scripts used to package CMake itself for distribution on ``cmake.org``.
    See `Utilities/Release/README.rst`_.

.. _`CMake Documentation Guide`: documentation.rst
.. _`Tests/README.rst`: ../../Tests/README.rst
.. _`Utilities/Release/README.rst`: ../../Utilities/Release/README.rst
