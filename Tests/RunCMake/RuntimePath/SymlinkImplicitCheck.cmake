file(COPY ${dir}/bin${cfg_dir}/exe DESTINATION ${dir})
file(RPATH_CHANGE FILE "${dir}/exe" OLD_RPATH "${dir}/libLink" NEW_RPATH "old-should-not-exist")
