.gitlab/ci/dpclang.sh
export PATH="$PWD/.gitlab/dpclang/bin:$PATH"
export LD_LIBRARY_PATH="$PWD/.gitlab/dpclang/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
dpclang --version
