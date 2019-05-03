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
