#!/usr/bin/perl -w
# $Id: mkdep.pl,v 1.5 2001-01-18 12:15:07 avn Exp $

use strict;
require "../mkutil.pl";

if ($#ARGV < 3) {
	print STDERR "Syntax: makedep.pl <mod_name> <depfile> <cdepfile> <modules>...\n";
	exit(1);
}

my $modname = shift(@ARGV);
my $depfile = shift(@ARGV);
my $cdepfile = shift(@ARGV);
my @modules = get_modules("../", @ARGV);
my %deps;

my $m;
foreach $m (@modules) {
	next if $m->{module} eq $modname;

	my $hfile = "/" . $m->{module} . ".h";
	open(IN, $depfile) || die "$!: can't open $depfile";
	while (<IN>) {
		next if (/^#/);

		if (/$hfile/) {
			$deps{$m->{module}} = 1;
		}
	}
	close(IN);
}

my @depslist = keys %deps;
print "Creating $cdepfile\n";
open(OUT, ">$cdepfile") || die "$!: can't open $cdepfile";
print OUT << "__END__";
const char _depend[] = "@depslist";
__END__
close(OUT);
