#!/usr/bin/perl

use strict;
use warnings;

local $| = 1;

print "Content-type: text/plain\n\n";

for ( my $i = 1 ; $i <= 10 ; $i++ ) {
        print "$i\n";
            sleep(1);
}

print "Done.";
