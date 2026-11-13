LINK_LANGUAGE
-------------

.. |genex-family| replace:: :ref:`genex-family-link-language-and-id`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<LINK_LANGUAGE>

.. versionadded:: 3.18

The link language of the target when evaluating link options. See :ref:`the
related boolean expression <Boolean LINK_LANGUAGE Generator Expression>`
``$<LINK_LANGUAGE:languages>`` for notes about the portability of this generator
expression.

.. note::

  This generator expression is not supported by the link libraries
  properties to avoid side-effects due to the double evaluation of
  these properties.

Syntax
^^^^^^

.. code-block:: cmake

  $<LINK_LANGUAGE:languages>

.. versionadded:: 3.18

``1`` when the language used for link step matches any of the comma-separated
entries in ``languages``, otherwise ``0``.  This expression may be used to
specify link libraries, link options, link directories and link dependencies of
a particular language in a target. For example:

.. code-block:: cmake

  add_library(api_C ...)
  add_library(api_CXX ...)
  add_library(api INTERFACE)
  target_link_options(api   INTERFACE $<$<LINK_LANGUAGE:C>:-opt_c>
                                      $<$<LINK_LANGUAGE:CXX>:-opt_cxx>)
  target_link_libraries(api INTERFACE $<$<LINK_LANGUAGE:C>:api_C>
                                      $<$<LINK_LANGUAGE:CXX>:api_CXX>)

  add_executable(myapp1 main.c)
  target_link_options(myapp1 PRIVATE api)

  add_executable(myapp2 main.cpp)
  target_link_options(myapp2 PRIVATE api)

This specifies to use the ``api`` target for linking targets ``myapp1`` and
``myapp2``. In practice, ``myapp1`` will link with target ``api_C`` and option
``-opt_c`` because it will use ``C`` as link language. And ``myapp2`` will link
with ``api_CXX`` and option ``-opt_cxx`` because ``CXX`` will be the link
language.

.. _`Constraints LINK_LANGUAGE Generator Expression`:

.. note::

  To determine the link language of a target, it is required to collect,
  transitively, all the targets which will be linked to it. So, for link
  libraries properties, a double evaluation will be done. During the first
  evaluation, ``$<LINK_LANGUAGE:..>`` expressions will always return ``0``.
  The link language computed after this first pass will be used to do the
  second pass. To avoid inconsistency, it is required that the second pass
  do not change the link language. Moreover, to avoid unexpected
  side-effects, it is required to specify complete entities as part of the
  ``$<LINK_LANGUAGE:..>`` expression. For example:

  .. code-block:: cmake

    add_library(lib STATIC file.cxx)
    add_library(libother STATIC file.c)

    # bad usage
    add_executable(myapp1 main.c)
    target_link_libraries(myapp1 PRIVATE lib$<$<LINK_LANGUAGE:C>:other>)

    # correct usage
    add_executable(myapp2 main.c)
    target_link_libraries(myapp2 PRIVATE $<$<LINK_LANGUAGE:C>:libother>)

  In this example, for ``myapp1``, the first pass will, unexpectedly,
  determine that the link language is ``CXX`` because the evaluation of the
  generator expression will be an empty string so ``myapp1`` will depends on
  target ``lib`` which is ``C++``. On the contrary, for ``myapp2``, the first
  evaluation will give ``C`` as link language, so the second pass will
  correctly add target ``libother`` as link dependency.
