#!/usr/bin/perl
$rep = $ARGV[0];
$str = "a"x$rep;
$re = ("a?"x$rep).$str;
print ($str =~ /$re/);

