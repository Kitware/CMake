message
-------

Display a message to the user.

.. code-block:: cmake

  message([<mode>] "message to display" ...)

The optional ``<mode>`` keyword determines the type of message:

``FATAL_ERROR``
  CMake Error, stop processing and generation.

``SEND_ERROR``
  CMake Error, continue processing, but skip generation.

``WARNING``
  CMake Warning, continue processing.

``AUTHOR_WARNING``
  CMake Warning (dev), continue processing.

``DEPRECATION``
  CMake Deprecation Error or Warning if variable
  :variable:`CMAKE_ERROR_DEPRECATED` or :variable:`CMAKE_WARN_DEPRECATED`
  is enabled, respectively, else no message.

(none) or ``NOTICE``
  Important message printed to stderr to attract user's attention.

``STATUS``
  The main interesting messages that project users might be interested in.
  Ideally these should be concise, no more than a single line, but still
  informative.

``VERBOSE``
  Detailed informational messages intended for project users.  These messages
  should provide additional details that won't be of interest in most cases,
  but which may be useful to those building the project when they want deeper
  insight into what's happening.

``DEBUG``
  Detailed informational messages intended for developers working on the
  project itself as opposed to users who just want to build it.  These messages
  will not typically be of interest to other users building the project and
  will often be closely related to internal implementation details.

``TRACE``
  Fine-grained messages with very low-level implementation details.  Messages
  using this log level would normally only be temporary and would expect to be
  removed before releasing the project, packaging up the files, etc.

The CMake command-line tool displays ``STATUS`` to ``TRACE`` messages on stdout
with the message preceded by two hyphens and a space.  All other message types
are sent to stderr and are not prefixed with hyphens.  The CMake GUI displays
all messages in its log area.  The interactive dialogs (:manual:`ccmake(1)`
and :manual:`cmake-gui(1)`) show ``STATUS`` to ``TRACE`` messages one at a
time on a status line and other messages in interactive pop-up boxes.
The ``--loglevel`` command-line option to each of these tools can be used to
control which messages will be shown.

CMake Warning and Error message text displays using a simple markup
language.  Non-indented text is formatted in line-wrapped paragraphs
delimited by newlines.  Indented text is considered pre-formatted.
