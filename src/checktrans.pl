#!/usr/bin/perl -w
#
# $Id: checktrans.pl,v 1.7 2001-01-23 21:22:18 fjoe Exp $
#
# Usage: checktrans.pl [-u] [-d] [-F] files...
# Options:
#	-u	- print unused messages from msgdb
#	-d	- debug
#	-F	- do not print untranslated messages by file
#

use strict;
use Getopt::Std;
use vars qw/ $opt_u $opt_d $opt_F /;

# path to msgdb
my $sog_root = "..";
my $checktrans_filename = "checktrans";
my $byfile_filename = "checktrans.byfile";

#
# messages to skip (skill names are added here)
my @skip_msgs = ( '', ' ', ' \'%s\'' );

# some common messages
push @skip_msgs, qw / Ok. all %s %s%c%s %d \n ]\n {x $t %s\n .\n auto ON OFF /;
push @skip_msgs, qw / none (none) /;
push @skip_msgs, qw / r w w+ a a+ /;

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
# messages to skip (messages which are keys of this hash are not checked
# for translation)
my %skip = map { $_ => 1 } @skip_msgs;

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
		next if (/(KEY|aff_fwrite_list|glob_lookup|str_printf|yyerror)\("/);
		next if (/(IS_TOKEN|db_set_arg|do_help|fwrite_string|fprintf|fwrite_word|is_name|mlstr_dump|mlstr_fwrite|log|str_cmp|str_cscmp|str_prefix|strcmp)\([&a-zA-Z_][a-zA-Z0-9_+->()]*, "/);
		next if (/dofun\("help", [a-zA-Z_][a-zA-Z0-9_]*, "/);
		next if (m|// notrans$|);

		my $seen_escape = 0;	# seen escape character ('\\')
		my $seen_quote = 0;	# seen quote character ('\'')

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
			} elsif ($line[$i] eq '"' && !$seen_escape && !$seen_quote) {
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
				my $str = join '', @string;
				push @{$file_strings{$str}}, $start_line
				    if !exists $skip{$str};
				$#string = -1;
				$seen_eos = 0;
			}
		} continue {
			$seen_quote = ($line[$i] eq '\'');
			$seen_escape = ($line[$i++] eq '\\') && !$seen_escape;
		}
	} continue {
		print STDERR "$linenum: ", join '', @line if $opt_d;
	}
	close(INPUT);

	my @strings = keys %file_strings;
	my $found = 0;
	foreach my $i (@strings) {
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
			if (!$opt_F) {
				if (!$found) {
					$found = 1;
					my $usage_count = $#strings + 1;
					print BYFILE "File $filename (${usage_count}):\n";
				}
				print BYFILE "\t[$i]: @{$file_strings{$i}}\n";
			}
		}
	}
	print BYFILE "\n" if ($found && !$opt_F);
}

sub usage_count
{
	my $file_ref = shift;
	my $count = 0;

	foreach my $f (keys %{$file_ref}) {
		$count += $#{$file_ref->{$f}};
	}
	return $count;
}

sub dump_messages
{
	my $title = shift;
	my $msgdb_ref = shift;

	#
	# sort by number of files (descending)
	my @strings = sort { usage_count(%{$msgdb_ref}->{$b}) <=> usage_count(%{$msgdb_ref}->{$a}) } keys %{$msgdb_ref};
	return if $#strings < 0;

	print CHECKTRANS "$title:\n";
	foreach my $i (@strings) {
		my $file_ref = %{$msgdb_ref}->{$i};
		print CHECKTRANS "[$i]\n";
		foreach my $f (keys %{$file_ref}) {
			print CHECKTRANS "\t$f: [@{$file_ref->{$f}}]\n";
		}
	}
	print CHECKTRANS "\n";
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

getopts('udF');

#
# read msgdb
my $msg = "";
my $search_rus = 0;		# we are searching for @rus
open(IN, "<$sog_root/etc/msgdb");
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

#
# read skills.conf (all skill names will be skipped)
open(IN, "<$sog_root/etc/skills.conf");
while (<IN>) {
	chomp;
	if (s/^Name (.*)~$/$1/) {
		s/\@eng (.*)\@rus.*/$1/;
		$skip{$_} = 1;
	}
}
close(IN);

#
# read cmd.conf (all command names will be skipped)
open(IN, "<$sog_root/etc/cmd.conf");
while (<IN>) {
	chomp;
	$skip{$_} = 1 if (s/^name (.*)~$/$1/);
}
close(IN);

#
# read cmd.conf (all command names will be skipped)
open(IN, "<$sog_root/etc/uhandlers.conf");
while (<IN>) {
	chomp;
	$skip{$_} = 1 if (s/^name (.*)~$/$1/);
}
close(IN);

#
# read specs
open(IN, "grep -h '^Name ' $sog_root/specs/*|");
while (<IN>) {
	chomp;
	$skip{$_} = 1 if (s/^Name (.*)~$/$1/);
}
close(IN);

#
# read damtype.conf (all damtype names will be skipped)
open(IN, "<$sog_root/etc/damtype.conf");
while (<IN>) {
	chomp;
	$skip{$_} = 1 if (s/^Name (.*)$/$1/);
}
close(IN);

#
# read materials.conf (all material names will be skipped
open(IN, "<$sog_root/etc/materials.conf");
while (<IN>) {
	chomp;
	$skip{$_} = 1 if (s/^Name (.*)~$/$1/);
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
# process input files and generate byfile statistics
rename($byfile_filename, "${byfile_filename}.old")
	if ( -f $byfile_filename );
open(BYFILE, ">$byfile_filename");

foreach my $i (@ARGV) {
	process_file($i);
}

close(BYFILE);

#
# generate overall statistics
rename($checktrans_filename, "${checktrans_filename}.old")
	if ( -f $checktrans_filename );
open(CHECKTRANS, ">$checktrans_filename");

if ($opt_u) {
	print CHECKTRANS "===> Unused messages:\n";
	foreach my $i (keys %msgdb) {
		next if (%{$msgdb{$i}});
		print CHECKTRANS "\t[$i]\n";
	}
	print CHECKTRANS "\n";
}

dump_messages("===> Messages used with '\\n' added", \%msgdb_nl);
dump_messages("===> Messages used with '\\n' stripped", \%msgdb_nonl);
dump_messages("===> Messages without translation", \%msgdb_notfound) if !$opt_F;

close(CHECKTRANS);
