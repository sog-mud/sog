#!/usr/bin/perl -w
use strict;
require "makeutil.pl";

my $module_decl = "module_decl";
my @modules;		# siz^H^H^Horder does matter

my $i = 0;
for $_ (@ARGV) {
	next if (/sog/);
	next if (/make-module-depend/);

	$modules[$i++] = mod_name($_);
}

my $module_decl_h = $module_decl . ".h";
my $h_id = "_" . uc($module_decl_h) . "_";
$h_id =~ s/\./_/g;

print "Creating $module_decl_h\n";
open(HOUT, ">$module_decl_h") || die "$!: can't open $module_decl_h";
print HOUT << "__END__";
#ifndef $h_id
#define $h_id

__END__

my $module_decl_c = $module_decl . ".c";
print "Creating $module_decl_c\n";
open(COUT, ">$module_decl_c") || die "$!: can't open $module_decl_c";
print COUT << "__END__";
#include <stdlib.h>
#include "typedef.h"
#include "$module_decl_h"
#include "flag.h"

flaginfo_t module_names[] =
{
	{ "",			TABLE_INTVAL			},

__END__

$i = 0;
for $_ (@modules) {
	my $mod_id = "MOD_" . uc($modules[$i]);
	my $mod_num = $i + 1;

	print HOUT << "__END__";
#define $mod_id		$mod_num
__END__

	print COUT << "__END__";
	{ "$modules[$i]",		$mod_id,		FALSE	},
__END__
	$i++;
}

print HOUT << "__END__";

#endif
__END__
close(HOUT);

print COUT << "__END__";

	{ NULL }
};
__END__
close(HOUT);
