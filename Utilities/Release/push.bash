#!/usr/bin/env bash

usage='usage: push.bash [<options>] [--] <dest>

Options:

    --dir     <dir>            Specify subdirectory under destination.
                               Defaults to "v<version>".
    --version <ver>            CMake <major>.<minor> version number to push.
                               Defaults to version of source tree.
'

die() {
    echo "$@" 1>&2; exit 1
}

cmake_source_dir="${BASH_SOURCE%/*}/../.."

cmake_version_component()
{
  sed -n "
/^set(CMake_VERSION_${1}/ {s/set(CMake_VERSION_${1} *\([0-9]*\))/\1/;p;}
" "${cmake_source_dir}/Source/CMakeVersion.cmake"
}


version=''
dir=''
while test "$#" != 0; do
    case "$1" in
    --dir) shift; dir="$1" ;;
    --version) shift; version="$1" ;;
    --) shift ; break ;;
    -*) die "$usage" ;;
    *) break ;;
    esac
    shift
done
test "$#" = 1 || die "$usage"
readonly dest="$1"

if test -z "$version"; then
    cmake_version_major="$(cmake_version_component MAJOR)"
    cmake_version_minor="$(cmake_version_component MINOR)"
    version="${cmake_version_major}.${cmake_version_minor}"
fi
readonly version

if test -z "$dir"; then
    dir="v${version}"
fi
readonly dir
if ! test -d "${dest}/${dir}"; then
    mkdir "${dest}/${dir}"
fi

for f in cmake-${version}*; do
    if ! test -f "${f}"; then
        continue
    fi

    echo "pushing '${f}'"

    # Make a copy with a new timestamp and atomically rename into place.
    tf="${dest}/${dir}/.tmp.${f}"
    df="${dest}/${dir}/${f}"
    cp "${f}" "${tf}"
    mv "${tf}" "${df}"

    # Pause to give each file a distinct time stamp even with 1s resolution
    # so that sorting by time also sorts alphabetically.
    sleep 1.1
done
