export HIPCXX=/usr/bin/clang++
export HIPFLAGS='--rocm-path=/usr --rocm-device-lib-path=/usr/lib64/rocm/llvm/lib/clang/22/lib/amdgcn/bitcode'

git apply --unsafe-paths --directory=/ .gitlab/ci/env_fedora45_hip_radeon.patch
ln -s /usr/lib64/libamdhip64.so /usr/lib/libamdhip64.so
