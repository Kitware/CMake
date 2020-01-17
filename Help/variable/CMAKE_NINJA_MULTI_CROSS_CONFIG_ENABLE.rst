CMAKE_NINJA_MULTI_CROSS_CONFIG_ENABLE
-------------------------------------

If this variable is enabled, cross-configuration building is enabled in the
:generator:`Ninja Multi-Config` generator. See the generator's description for
more details. This variable is ``OFF`` by default.

This variable is meant to be set from the command line (via
``-DCMAKE_NINJA_MULTI_CROSS_CONFIG_ENABLE:BOOL=ON``) and should not be set from
project code.
