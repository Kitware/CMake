source .gitlab/ci/env_intelcompiler_license.sh

if test -r /opt/intel/oneapi/setvars.sh; then
  source /opt/intel/oneapi/setvars.sh
fi

export CC=icx CXX=icpx FC=ifx
