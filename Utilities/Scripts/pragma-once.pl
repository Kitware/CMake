#!/usr/bin/perl

use strict;
use warnings;

use constant {
  START => 0,
  HAVE_IFNDEF => 1,
  HAVE_DEFINE => 2,
  HAVE_ENDIF => 3,
  HAVE_PRAGMA_ONCE => 4,
};
my $state = START;
my $blank_count = 0;
my $endif = '';
while (<>) {
  if ($state == START) {
    if (/^#ifndef [a-zA-Z_][a-zA-Z0-9_]*$/) {
      $state = HAVE_IFNDEF;
      print "#pragma once\n";
    } else {
      if (/^#pragma once$/) {
        $state = HAVE_PRAGMA_ONCE;
      }
      print;
    }
  } elsif ($state == HAVE_IFNDEF) {
    if (/^#define [a-zA-Z_][a-zA-Z0-9_]*$/) {
      $blank_count = 0;
      $state = HAVE_DEFINE;
    } else {
      print;
    }
  } elsif ($state == HAVE_DEFINE) {
    if (/^#endif/) {
      $endif = $_;
      $state = HAVE_ENDIF;
    } elsif (/^$/) {
      $blank_count++;
    } else {
      for (my $i = 0; $i < $blank_count; $i++) {
        print "\n";
      }
      $blank_count = 0;
      print;
    }
  } elsif ($state == HAVE_ENDIF) {
    for (my $i = 0; $i < $blank_count; $i++) {
      print "\n";
    }
    $blank_count = 0;
    print $endif;
    $state = HAVE_DEFINE;
    if (/^#endif/) {
      $endif = $_;
      $state = HAVE_ENDIF;
    } elsif (/^$/) {
      $blank_count++;
    } else {
      print;
    }
  } elsif ($state == HAVE_PRAGMA_ONCE) {
    print;
  }
  if (eof) {
    if ($state != HAVE_ENDIF && $state != HAVE_PRAGMA_ONCE) {
      print STDERR "Malformed header file: $ARGV\n";
      exit 1;
    }
    $state = START;
    $blank_count = 0;
  }
}
