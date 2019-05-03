FindBoost-fphsa
---------------

* The :module:`FindBoost` module was reworked to expose a more
  consistent user experience between its config and module modes
  and with other find modules in general.

  * A new imported target ``Boost::headers`` is now defined (same
    as ``Boost::boost``).

  * New output variables ``Boost_VERSION_MACRO``,
    ``Boost_VERSION_MAJOR``, ``Boost_VERSION_MINOR``,
    ``Boost_VERSION_PATCH``, and ``Boost_VERSION_COUNT``
    were added.

  * The internal logic for determining the value for
    ``Boost_FOUND``, for version and component checks, and
    for reporting the result to the user was replaced with
    the :module:`FindPackageHandleStandardArgs` module. (This
    fixed a bug that sometimes printed wrong status
    messages in config mode.)

  * The ``QUIET`` argument passed to :command:`find_package` is no
    longer ignored in config mode.

  * *Known issue*: The CMake package shipped with Boost ``1.70.0``
    ignores the ``QUIET`` argument passed to :command:`find_package`.
    This is fixed in the next Boost release.

  * The input switch ``Boost_DETAILED_FAILURE_MSG`` was
    removed.

  * ``Boost_VERSION`` now reports the version in ``x.y.z``
    format in module mode.  See policy :policy:`CMP0093`.
