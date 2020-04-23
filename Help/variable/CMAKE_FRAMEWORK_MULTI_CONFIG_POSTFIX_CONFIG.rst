CMAKE_FRAMEWORK_MULTI_CONFIG_POSTFIX_<CONFIG>
---------------------------------------------

Default framework filename postfix under configuration ``<CONFIG>`` when
using a multi-config generator.

When a framework target is created its :prop_tgt:`FRAMEWORK_MULTI_CONFIG_POSTFIX_<CONFIG>`
target property is initialized with the value of this variable if it is set.
