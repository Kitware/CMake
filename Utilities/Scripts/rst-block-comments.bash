#!/bin/bash

files="$(git ls-files Modules | grep -v ' ' | xargs grep -l '^#.rst:$')"

perl -i -e '
  use strict;
  use warnings;

  my $rst = 0;
  while (<>) {
    if ($rst) {
      if (/^# (.*)$/) {
        print "$1\n";
      } elsif (/^#$/) {
        print "\n";
      } else {
        $rst = 0;
        print "#]=======================================================================]\n";
        print $_;
      }
    } elsif (/^#\.rst:$/) {
      $rst = 1;
      print "#[=======================================================================[.rst:\n";
    } else {
      print $_;
    }

    if ($rst && eof) {
      $rst = 0;
      print "#]=======================================================================]\n";
    }
  }
' $files
