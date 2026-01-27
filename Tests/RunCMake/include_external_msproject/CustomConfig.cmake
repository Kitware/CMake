include_external_msproject(external external.project
                           GUID aaa-bbb-ccc-000)
set_target_properties(external PROPERTIES
  MAP_IMPORTED_CONFIG_RELEASE        "Custom - Release"
  MAP_IMPORTED_CONFIG_MINSIZEREL     "Custom - MinSizeRel"
  MAP_IMPORTED_CONFIG_RELWITHDEBINFO "Custom - RelWithDebInfo"
)
