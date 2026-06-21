#!/bin/sh

static=false
mode=
pkg=

for arg in "$@"; do
  case "$arg" in
    --version)
      echo "1.0"
      exit 0
      ;;
    --static)
      static=true
      ;;
    --modversion)
      mode=modversion
      ;;
    --variable=*)
      mode=variable
      variable="${arg#--variable=}"
      ;;
    --cflags)
      mode=cflags
      ;;
    --libs)
      mode=libs
      ;;
    --print-errors|--short-errors)
      ;;
    *)
      pkg="$arg"
      ;;
  esac
done

case "$mode:$pkg" in
  modversion:good|modversion:bad)
    echo "1.0"
    echo "/fake-prefix"
    ;;
  variable:good|variable:bad)
    echo "/fake-$variable"
    ;;
  cflags:good|cflags:bad)
    echo "-I/fake-include"
    ;;
  libs:good)
    if $static; then
      echo "-lgood -ldep"
    else
      echo "-lgood"
    fi
    ;;
  libs:bad)
    if $static; then
      exit 1
    else
      echo "-lbad"
    fi
    ;;
  *)
    exit 1
    ;;
esac
