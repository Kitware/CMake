JOB_SERVER_AWARE
----------------

.. versionadded:: 4.5

``JOB_SERVER_AWARE`` is a boolean specifying that the commands produced by the
instantiation of the rule are GNU Make job server aware.

For the :generator:`Unix Makefiles`, :generator:`MSYS Makefiles`, and
:generator:`MinGW Makefiles` generators this will add the ``+`` prefix to the
recipe line. See the `GNU Make Documentation`_ for more information.

This option is ignored by other generators.

.. _`GNU Make Documentation`: https://www.gnu.org/software/make/manual/html_node/MAKE-Variable.html
