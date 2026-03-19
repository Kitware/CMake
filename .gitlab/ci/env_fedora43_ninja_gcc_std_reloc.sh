# "Misplace" the `libstdc++.modules.json` file so that
# `CMAKE_CXX_STDLIB_MODULES_JSON` is needed to use `import std`.
stdlib_modules_dir="/usr/lib/gcc/x86_64-redhat-linux/15"
mkdir "$stdlib_modules_dir.reloc"
mv "$stdlib_modules_dir/libstdc++.modules.json" "$stdlib_modules_dir.reloc"
export CMAKE_CI_CXX_STDLIB_MODULES_JSON="$stdlib_modules_dir.reloc/libstdc++.modules.json"
unset stdlib_modules_dir
