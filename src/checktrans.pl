#!/usr/bin/perl -w
#
# $Id: checktrans.pl,v 1.4 2001-01-18 22:15:49 fjoe Exp $
#
# Usage: checktrans.pl [-u] [-d] files...
# Options:
#	-u	- do not print unused messages from msgdb
#	-d	- debug
#

use strict;
use Getopt::Std;
use vars qw/ $opt_u $opt_d /;

# path to msgdb
my $msgdb_path = "../etc/msgdb";
#my $msgdb_path = "foo";

#
# msgdb: msgdb (read from <STDIN>)
#
# key	- msg
# value	- hash of filenames:
#		key - file name
#		value - array of line numbers
my %msgdb;

#
# msgdb_nl: messages used in sources with 'nl' added (same as %msgdb)
my %msgdb_nl;

#
# msgdb_nonl: messages used in sources with 'nl' stripped (same as %msgdb)
my %msgdb_nonl;

#
# notfound: messages not found in sources without translation (same as %msgdb)
my %msgdb_notfound;

#
# key value is hash
sub process_file
{
	my $filename = shift;
	my $linenum = 0;

	print STDERR "Processing file $filename\n";
	open(INPUT, $filename) || die "$filename: $!\n";
	my @line;

	#
	# key - string
	# value - array of line numbers
	my %file_strings;

	#
	# current string attributes
	my @string;
	my $start_line;
	my $start_pos;

	#
	# parse flags
	my $search_eoc = 0;		# we are searching end of
					# multiline comments
	my $search_eos = 0;		# we are searching end of string
	my $seen_eos = 0;		# we just finished parsing a string

	READ_LINE: while (<INPUT>) {
		$linenum++;

		#
		# skip macro processor directives
		next if (/^#/);

		# skip some specific strings
		next if (/(KEY|aff_write_list|skill_beats|str_printf|yyerror)\("/);
		next if (/(IS_TOKEN|check_improve|db_set_arg|do_help|fwrite_string|fprintf|fwrite_word|get_skill|mlstr_dump|mlstr_fwrite|log|skill_mana|str_cmp|str_cscmp|strcmp)\([a-zA-Z_][a-zA-Z0-9_+()]*, "/);
		next if (/dofun\("help", [a-zA-Z_][a-zA-Z0-9_]*, "/);
		next if (m|// notrans$|);

		my $seen_escape = 0;	# seen escape character ('\\')
		my $i = 0;
		@line = split //;
		PARSE_LINE: while ($i <= $#line) {
			#
			# check for comments
			if ($search_eoc) {
				#
				# check for end of multiline comments
				if ($line[$i] eq "*" && $line[$i+1] eq "/") {
					print STDERR "end of multiline comments $linenum:$i\n" if $opt_d;
					$i++;

					$search_eoc = 0;
					next PARSE_LINE;
				}
				next PARSE_LINE;
			} elsif ($line[$i] eq "/") {
				#
				# check for '//' comments
				last PARSE_LINE if ($line[$i+1] eq "/");

				#
				# check for start of multiline comments
				if ($line[$i+1] eq "*") {
					$i++;

					$search_eoc = 1;
					next PARSE_LINE;
				}
			}

			if ($search_eos) {
				#
				# check for end of string
				if ($line[$i] eq '"' && !$seen_escape) {
					#
					# eos found. we delay finilazing
					# the string because there can be
					# string continuation just after some
					# space characters
					$search_eos = 0;
					$seen_eos = 1;
					print STDERR "supposed end of string: $linenum:$i\n" if $opt_d;
					next PARSE_LINE;
				}

				#
				# append character to current string
				push @string, $line[$i];
				next PARSE_LINE;
			} elsif ($line[$i] eq '"' && !$seen_escape) {
				if (!$seen_eos) {
					#
					# is not continuation
					print STDERR "start of string: $linenum:$i\n" if $opt_d;
					$start_line = $linenum;
					$start_pos = $i;
				}

				#
				# just a continuation
				$search_eos = 1;
				next PARSE_LINE;
			}

			if ($seen_eos && index("\n\r\t ", $line[$i]) < 0) {
				#
				# we just found an eos and it is not
				# space character
				print STDERR "end of string: $linenum:$i\n" if $opt_d;
				push @{$file_strings{join '', @string}}, $start_line;
				$#string = -1;
				$seen_eos = 0;
			}
		} continue {
			$seen_escape = ($line[$i++] eq '\\') && !$seen_escape;
		}
	} continue {
		print STDERR "$linenum: ", join '', @line if $opt_d;
	}

	print STDERR "All the strings:\n" if $opt_d;
	foreach my $i (keys %file_strings) {
		print STDERR "string: $i, line(s): @{$file_strings{$i}}\n" if $opt_d;
		my $nonl = $i;
		$nonl =~ s/\\n$//;
		if (exists $msgdb{$i}) {
			%{$msgdb{$i}}->{$filename} = \@{$file_strings{$i}};
		} elsif (exists $msgdb{$i . '\n'}) {
			%{$msgdb_nonl{$i . '\n'}}->{$filename} = \@{$file_strings{$i}};
		} elsif (exists $msgdb{$nonl}) {
			%{$msgdb_nl{$nonl}}->{$filename} = \@{$file_strings{$i}};
		} else {
			%{$msgdb_notfound{$i}}->{$filename} = \@{$file_strings{$i}};
		}
	}
	close(INPUT);
}

sub dump_messages
{
	my $title = shift;
	my $msgdb_ref = shift;

	print "$title:\n";
	foreach my $i (keys %{$msgdb_ref}) {
		print "[$i]\n";
		foreach my $f (keys %{%{$msgdb_ref}->{$i}}) {
			print "\t$f: [@{%{$msgdb_ref}->{$i}->{$f}}]\n";
		}
	}
	print "\n";
}

sub add_message
{
	my $msg = shift;

	$msg =~ s/~~/~/g;
	$msg =~ s/^\.//;
	$msgdb{$msg} = {};

	return 0;
}

#---------------------------------------------------------------------
# main program
#

getopts('ud');

#
# read msgdb from STDIN
my $msg = "";
my $search_rus = 0;		# we are searching for @rus
open(IN, "<$msgdb_path");
while (<IN>) {
	chomp;
	last if (/^\$~$/);
	if (s/^\@eng //) {
		if ($search_rus) {
			# we are still searching for '@rus '
			# save old message
			$search_rus = add_message($msg);
			print STDERR "Found \@rus, saved message without translation: '$msg'\n" if $opt_d;
		}

		# save current message
		if (s/(.*)\@rus .*/$1/) {
			# found and stripped @rus
			add_message($_);
			print STDERR "Saved one-line message: '$_'\n" if $opt_d;
			next;
		}

		print STDERR "Found new message start: '$_'\n" if $opt_d;
		$msg = $_ . "\\n";
		$search_rus = 1;
		next;
	} elsif ($search_rus) {
		if (s/(.*)\@rus .*/$1/) {
			# we are searching for '@rus ' and have found it
			$search_rus = add_message($msg . $_);
			print STDERR "Found \@rus, saved message: '$msg'\n" if $opt_d;
			next;
		}
		$msg = $msg . $_ . "\\n";
		next;
	}
}
close(IN);

if ($opt_d) {
	my @msgdb_eng = keys %msgdb;
	print STDERR "All english msgdb messages:\n";
	$" = "\n";
	print STDERR "@msgdb_eng\n";
	$" = " ";
}

#
# process input files
foreach my $i (@ARGV) {
	process_file($i);
}

if ($opt_u) {
	print "===> Unused messages:\n";
	foreach my $i (keys %msgdb) {
		next if (%{$msgdb{$i}});
		print "\t'$i'\n";
	}
	print "\n";
}

dump_messages("===> Messages used with '\\n' added", \%msgdb_nl);
dump_messages("===> Messages used with '\\n' stripped", \%msgdb_nonl);
dump_messages("===> Messages without translation", \%msgdb_notfound);
