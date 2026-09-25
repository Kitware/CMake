RESOURCE_ERROR_ACTION
---------------------

.. versionadded:: 4.5

This property defines the behavior in case the resource(s) requested via the
:prop_test:`RESOURCE_GROUPS` property cannot be satisfied because the resource
group is not available at all or not in sufficient amounts.

If the property is unset or set to ``FAIL`` the test will be marked as failed.
If the property is set to ``SKIP`` the test will be marked as skipped.

This setting takes precedence over :variable:`CTEST_RESOURCE_ERROR_ACTION`.
