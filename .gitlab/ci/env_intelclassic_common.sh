source .gitlab/ci/env_intelcompiler_license.sh

if test -r /opt/intel/oneapi/setvars.sh; then
  source /opt/intel/oneapi/setvars.sh
elif test -r /opt/intel/bin/compilervars.sh; then
  source /opt/intel/bin/compilervars.sh intel64
fi

export CC=icc CXX=icpc FC=ifort
