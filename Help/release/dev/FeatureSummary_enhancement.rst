FeatureSummary_enhancement
--------------------------

* The :command:`set_package_info`, :command:`set_feature_info`,
  :command:`print_enabled_features` and :command:`print_disabled_features`
  commands from the the :module:`FeatureSummary` module are now deprecated.

* The :command:`set_package_properties` command no longer forces the package
  type to ``OPTIONAL`` when the type is not explicitly set.

* The :command:`feature_summary` command in the :module:`FeatureSummary` module
  accepts the new ``QUIET_ON_EMPTY`` option that will suppresses the output when
  the list of packages that belong to the selected category is empty.

* The :command:`add_feature_info` in the :module:`FeatureSummary` module learned
  to accept lists of dependencies for deciding whether a feature is enabled or
  not.

* The package types accepted by the the :module:`FeatureSummary` module can now
  be tweaked by changing the :variable:`FeatureSummary_PKG_TYPES`,
  :variable:`FeatureSummary_REQUIRED_PKG_TYPES` and
  :variable:`FeatureSummary_DEFAULT_PKG_TYPE` global properties.
