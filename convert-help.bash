#!/usr/bin/env bash

if test $# -ne 1; then
  echo 1>&2 'Specify cmake executable directory'
  exit 1
fi &&
bin="$1" &&

# Extract .rst documentation and generate man section 1 pages
mkdir -p Help/manual &&
cd Help &&
mkdir tmp && cd tmp &&
"$bin"/cmake --help-full ../manual/cmake.1.rst &&
tar c * | (cd .. && tar x) &&
cd .. && rm -rf tmp &&
sed -i '1 i\
cmake(1)\
********\

' manual/cmake.1.rst &&
mkdir tmp && cd tmp &&
"$bin"/ctest --help-full ../manual/ctest.1.rst &&
mv command/ctest_*.rst ../command &&
cd .. && rm -rf tmp &&
sed -i '1 i\
ctest(1)\
********\

' manual/ctest.1.rst &&
mkdir tmp && cd tmp &&
"$bin"/cpack --help-full ../manual/cpack.1.rst &&
mv variable ../var_cpack &&
cd .. && rm -rf tmp &&
sed -i '1 i\
cpack(1)\
********\

' manual/cpack.1.rst &&
mkdir tmp && cd tmp &&
"$bin"/ccmake --help-full ../manual/ccmake.1.rst &&
cd .. && rm -rf tmp &&
sed -i '1 i\
ccmake(1)\
*********\

' manual/ccmake.1.rst &&
mkdir tmp && cd tmp &&
"$bin"/cmake-gui --help-full ../manual/cmake-gui.1.rst &&
cd .. && rm -rf tmp &&
sed -i '1 i\
cmake-gui(1)\
************\

' manual/cmake-gui.1.rst &&

# Remove trailing whitespace and blank lines
find . -name '*.rst' |
while read f; do
  sed -e 's/[ \t]*$//' -i "$f" &&
  sed -e ':a' -e '/^\n*$/ {$d;N;ba;}' -i "$f"
done

# Generate man section 7 pages
{
deprecated_commands=$(
cat <<EOF
   /command/build_name
   /command/exec_program
   /command/export_library_dependencies
   /command/install_files
   /command/install_programs
   /command/install_targets
   /command/link_libraries
   /command/make_directory
   /command/output_required_files
   /command/remove
   /command/subdir_depends
   /command/subdirs
   /command/use_mangled_mesa
   /command/utility_source
   /command/variable_requires
   /command/write_file
EOF
) &&
cat <<EOF &&
cmake-commands(7)
*****************

.. only:: html or latex

   .. contents::

Normal Commands
===============

These commands may be used freely in CMake projects.

.. toctree::
EOF
echo "$deprecated_commands" > tmp &&
ls command/*.rst |sort|sed 's|^|   /|;s|\.rst$||' |
grep -v /command/ctest_ | grep -v -x -F -f tmp &&
rm tmp &&
cat <<EOF &&

Deprecated Commands
===================

These commands are available only for compatibility with older
versions of CMake.  Do not use them in new code.

.. toctree::
$deprecated_commands

CTest Commands
==============

These commands are available only in ctest scripts.

.. toctree::
EOF
ls command/*.rst |sort|sed 's|^|   /|;s|\.rst$||' | grep /command/ctest_
} > manual/cmake-commands.7.rst &&
{
cat <<EOF &&
cmake-generators(7)
*******************

.. only:: html or latex

   .. contents::

All Generators
==============

.. toctree::
EOF
ls generator/*.rst |sort|sed 's|^|   /|;s|\.rst$||'
} > manual/cmake-generators.7.rst &&
{
cat <<EOF &&
cmake-modules(7)
****************

.. only:: html or latex

   .. contents::

All Modules
===========

.. toctree::
EOF
ls module/*.rst    |sort|sed 's|^|   /|;s|\.rst$||'
} > manual/cmake-modules.7.rst &&
{
cat <<EOF &&
cmake-policies(7)
*****************

.. only:: html or latex

   .. contents::

All Policies
============

.. toctree::
EOF
ls policy/*.rst    |sort|sed 's|^|   /|;s|\.rst$||'
} > manual/cmake-policies.7.rst &&
{
cat <<EOF &&
cmake-properties(7)
*******************

.. only:: html or latex

   .. contents::

Properties of Global Scope
==========================

.. toctree::
EOF
ls prop_gbl/*.rst  |sort|sed 's|^|   /|;s|\.rst$||' &&
cat <<EOF &&

Properties on Directories
=========================

.. toctree::
EOF
ls prop_dir/*.rst  |sort|sed 's|^|   /|;s|\.rst$||' &&
cat <<EOF &&

Properties on Targets
=====================

.. toctree::
EOF
ls prop_tgt/*.rst  |sort|sed 's|^|   /|;s|\.rst$||' &&
cat <<EOF &&

Properties on Tests
===================

.. toctree::
EOF
ls prop_test/*.rst |sort|sed 's|^|   /|;s|\.rst$||' &&
cat <<EOF &&

Properties on Source Files
==========================

.. toctree::
EOF
ls prop_sf/*.rst   |sort|sed 's|^|   /|;s|\.rst$||' &&
cat <<EOF &&

Properties on Cache Entries
===========================

.. toctree::
EOF
ls prop_cache/*.rst|sort|sed 's|^|   /|;s|\.rst$||'
} > manual/cmake-properties.7.rst &&
{
cat <<EOF &&
cmake-variables(7)
******************

.. only:: html or latex

   .. contents::

Variables that Provide Information
==================================

.. toctree::
EOF
ls var_info/*.rst  |sort|sed 's|^|   /|;s|var_info/|variable/|;s|\.rst$||' &&
cat <<EOF &&

Variables that Change Behavior
==============================

.. toctree::
EOF
ls var_cmake/*.rst |sort|sed 's|^|   /|;s|var_cmake/|variable/|;s|\.rst$||' &&
cat <<EOF &&

Variables that Describe the System
==================================

.. toctree::
EOF
ls var_sys/*.rst   |sort|sed 's|^|   /|;s|var_sys/|variable/|;s|\.rst$||' &&
cat <<EOF &&

Variables that Control the Build
================================

.. toctree::
EOF
ls var_build/*.rst |sort|sed 's|^|   /|;s|var_build/|variable/|;s|\.rst$||' &&
cat <<EOF &&

Variables for Languages
=======================

.. toctree::
EOF
ls var_lang/*.rst  |sort|sed 's|^|   /|;s|var_lang/|variable/|;s|\.rst$||' &&
cat <<EOF &&

Variables for CPack
===================

.. toctree::
EOF
ls var_cpack/*.rst |sort|sed 's|^|   /|;s|var_cpack/|variable/|;s|\.rst$||'
} > manual/cmake-variables.7.rst &&
mkdir variable &&
mv var_*/* variable &&
rmdir var_* &&
cd .. &&

# Move module help back into .cmake module file comments
(cd Help/module && ls *.rst) |
while read m; do
  dm="Help/module/$m" &&
  cm="Modules/${m%.rst}.cmake" &&
  {
    echo '#.rst:' &&
    sed -e '
    /^./ s/^/# /
    /^$/ c #
    s/  *$//
    ' "$dm" &&
    echo '' &&
    sed -e '1,/^$/d' "$cm"
  } >"$cm.new" &&
  mv "$cm.new" "$cm" &&
  echo ".. cmake-module:: ../../$cm" > "$dm"
done
