#!/usr/bin/perl -w
use strict;
require "makeutil.pl";

my @modules = get_modules(@ARGV);
my $mod;

foreach $mod (@modules) {
	# init deps
	my %empty;
	$mod->{deps} = \%empty;

	my $m;
	my $hfile = "/" . $mod->{module} . ".h";
	foreach $m (@modules) {
		next if $m->{module} eq $mod->{module};

		my $inname = $m->{dir} . "/" . ".depend";
		open(IN, $inname) || die "$!: can't open $inname";

		while (<IN>) {
			next if (/^#/);

			if (/$hfile/) {
				${$m->{deps}}{$mod->{module}} = 1;
			}
		}
	}

	close(IN);
}

foreach $mod (@modules) {
	my @deps = keys %{$mod->{deps}};

	my $outname = $mod->{dir} . "/" . "_depend.c";
	if ($#deps >= 0) {
		print "Creating $outname\n";
		open(OUT, ">$outname") || die "$!: can't open $outname";
		print OUT << "__END__";
const char *_depend = "@deps";
__END__
		close(OUT);
	} elsif (-f $outname) {
		print "Deleting $outname\n";
		unlink($outname);
	}
}
