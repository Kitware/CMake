if test "$CMAKE_CI_NIGHTLY" = "true"; then
  source .gitlab/ci/acpp-env.sh
  source .gitlab/ci/android-ndk-env.sh
  source .gitlab/ci/android-sdk-env.sh
  source .gitlab/ci/ispc-env.sh
fi

# Test in a Latin-1 locale.
export LANG=en_US.ISO-8859-1

#FIXME(#28215): FindHDF5 module and/or tests need updates for Fedora 45+.
rm -rf /usr/lib64/cmake/hdf5 \
       /usr/lib64/mpich/lib/cmake/hdf5 \
       /usr/lib64/openmpi/lib/cmake/hdf5
