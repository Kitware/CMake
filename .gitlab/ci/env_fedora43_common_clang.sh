export CC=/usr/bin/clang-21
export CXX=/usr/bin/clang++-21
export FC=/usr/bin/flang-21

# FIXME(Fedora): Flang 21 packages do not make the runtime library findable.
# https://github.com/llvm/llvm-project/issues/138340
# https://bugzilla.redhat.com/show_bug.cgi?id=2401176
# https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1117534
export LD_LIBRARY_PATH=/usr/lib/clang/21/lib/x86_64-redhat-linux-gnu
