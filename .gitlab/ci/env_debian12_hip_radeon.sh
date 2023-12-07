export HIPCXX=/usr/bin/clang++-15
export HIPFLAGS='--rocm-path=/usr --rocm-device-lib-path=/usr/lib/x86_64-linux-gnu/amdgcn/bitcode'

# FIXME(debian): Clang is supposed to automatically parse a HIP version file.
# The ROCm installer places it at '$prefix/bin/.hipVersion', but the package
# on Debian moves it to '$prefix/share/hip/version'.  llvm-toolchain package
# version 15.0.7-4 has 'debian/patches/amdgpu/usr-search-paths.patch' for this,
# but Debian 12 currently provides version 15.0.6-4 without the patch.
export HIPFLAGS="$HIPFLAGS --hip-version=5.2"
