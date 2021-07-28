if test -n "$CMAKE_CI_INTELCOMPILER_LICENSE"; then
  if test -d /opt/intel/licenses; then
    mv "$CMAKE_CI_INTELCOMPILER_LICENSE" /opt/intel/licenses/ci.lic
  else
    rm "$CMAKE_CI_INTELCOMPILER_LICENSE"
  fi
  unset CMAKE_CI_INTELCOMPILER_LICENSE
fi
