#!/usr/bin/env bash

set -eu

if test "$#" -eq 4 &&
   test "$1" = "--no-weak" &&
   test "$2" = "--defined-only" &&
   test "$3" = "--format=posix"; then
  print_file_name=0
  input=$4
elif test "$#" -eq 5 &&
     test "$1" = "--no-weak" &&
     test "$2" = "--defined-only" &&
     test "$3" = "--format=posix" &&
     test "$4" = "--print-file-name"; then
  print_file_name=1
  input=$5
else
  echo "unexpected nm arguments: $*" >&2
  exit 1
fi

pending_symbol=0

finish_symbol()
{
  if test "${pending_symbol}" -eq 1; then
    printf ' 0 0\r\n'
    pending_symbol=0
  fi
}

start_symbol()
{
  finish_symbol
  if test "${print_file_name}" -eq 1; then
    printf '%s: %s %s' "$1" "$2" "$3"
  else
    printf '%s %s' "$2" "$3"
  fi
  pending_symbol=1
}

process_object()
{
  object=$1
  case "${object}" in
    \"*\")
      object=${object#\"}
      object=${object%\"}
      ;;
  esac

  case "${object}" in
    *single.obj)
      start_symbol "${object}" single T
      ;;
    *symbol-types.obj)
      start_symbol "${object}" absolute A
      start_symbol "${object}" bss B
      start_symbol "${object}" common C
      start_symbol "${object}" data D
      start_symbol "${object}" import I
      start_symbol "${object}" debug N
      start_symbol "${object}" readonly R
      start_symbol "${object}" '??_7Example@@6B@' R
      start_symbol "${object}" '??_R4Example@@6B@' R
      start_symbol "${object}" section S
      start_symbol "${object}" text T
      start_symbol "${object}" undefined U
      start_symbol "${object}" weak_object V
      start_symbol "${object}" weak W
      start_symbol "${object}" local_absolute a
      start_symbol "${object}" local_bss b
      start_symbol "${object}" local_common c
      start_symbol "${object}" local_data d
      start_symbol "${object}" local_import i
      start_symbol "${object}" local_debug n
      start_symbol "${object}" local_readonly r
      start_symbol "${object}" local_section s
      start_symbol "${object}" local_text t
      start_symbol "${object}" local_weak_object v
      start_symbol "${object}" local_weak w
      start_symbol "${object}" unknown '?'
      ;;
    *first.obj)
      start_symbol "${object}" first T
      ;;
    *no-symbols.obj)
      sleep 1
      printf '%s: no symbols\n' "${object}" >&2
      sleep 1
      finish_symbol
      ;;
    *second.obj)
      start_symbol "${object}" second T
      ;;
    *spaced.obj)
      start_symbol "${object}" spaced T
      ;;
    *mixed.obj)
      start_symbol "${object}" mixed T
      ;;
    *)
      echo "unexpected nm input: ${object}" >&2
      exit 1
      ;;
  esac
}

case "${input}" in
  @*)
    response=${input#@}
    while IFS= read -r object || test -n "${object}"; do
      process_object "${object}"
    done < "${response}"
    ;;
  *)
    process_object "${input}"
    ;;
esac
finish_symbol
