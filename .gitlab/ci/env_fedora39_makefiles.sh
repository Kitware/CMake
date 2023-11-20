if test "$CMAKE_CI_NIGHTLY" = "true"; then
  source .gitlab/ci/ispc-env.sh
fi

# Patch HDF5 Fortran compiler wrappers to work around Fedora bug.
# https://bugzilla.redhat.com/show_bug.cgi?id=2183289
sed -i '/^includedir=/ s|/mpich-x86_64||'   /usr/lib64/mpich/bin/h5pfc
sed -i '/^includedir=/ s|/openmpi-x86_64||' /usr/lib64/openmpi/bin/h5pfc
