#!/usr/bin/perl -w
use strict;

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
	my $end_line;
	my $end_pos;

	#
	# parse flags
	my $search_comments_end = 0;
	my $parse_string = 0;
	my $seen_something = 1;

	READ_LINE: while (<INPUT>) {
		$linenum++;
		@line = split //;

		# skip macro processor directives
		next if (/^#/);

		my $prev_char;
		my $i = 0;
		PARSE_LINE: while ($i <= $#line) {
			#
			# check for comments
			#
			if ($search_comments_end) {
				if ($line[$i] eq "*" && $line[$i+1] eq "/") {
#					print "$linenum: end of multiline comments\n";
					$i++;

					$search_comments_end = 0;
					next PARSE_LINE;
				}
				next PARSE_LINE;
			} elsif ($line[$i] eq "/") {
				last PARSE_LINE if ($line[$i+1] eq "/");
				if ($line[$i+1] eq "*") {
					$i++;

					$search_comments_end = 1;
					next PARSE_LINE;
				}
			}

			if ($parse_string) {
				if ($line[$i] eq '"') {
					$parse_string = 0;
					next PARSE_LINE;
				} elsif ($line[$i] eq "\\" && $line[$i+1]) {
					push @string, $line[$i++];
				}

				push @string, $line[$i];
				next PARSE_LINE;
			} elsif ($line[$i] eq '"') {
				if ($seen_something) {
#					print "Start of string: $linenum:$i\n";
					$start_line = $linenum;
					$start_pos = $i;
					$seen_something = 0;
				}

				$parse_string = 1;
				next PARSE_LINE;
			}

			if (index("\n\r\t ", $line[$i]) < 0
			&&  !$seen_something) {
#				print "End of string: $linenum:$i\n";
				$end_line = $linenum;
				$end_pos = $i;

				push @{$file_strings{join '', @string}}, $start_line;
				$#string = -1;
				$seen_something = 1;
			}
		} continue {
			$prev_char = $line[$i];
			$i++;
		}
	} continue {
#		print "$linenum: ", join '', @line;
	}

#	print "All the strings:\n";
	foreach my $i (keys %file_strings) {
#		print "string: $i, line(s): @{$file_strings{$i}}\n";
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

#
# read msgdb from STDIN
my $msg = "";
my $search_rus = 0;
open(IN, "<$msgdb_path");
while (<IN>) {
	chomp;
	last if (/^\$~$/);
	if ($search_rus) {
		if (s/(.*)\@rus .*/$1/) {
			# we are searching for '@rus ' and have found it
			$msg = $msg . $_;
			$msg =~ s/~~/~/g;
			$msg =~ s/^\.//;
			$msgdb{$msg} = {};
#			print "Found \@rus, saved message: '$msg'\n";

			$search_rus = 0;
			next;
		}
		$msg = $msg . $_ . "\\n";
		next;
	}
	if (s/^\@eng //) {
		# save current message
		if (s/(.*)\@rus .*/$1/) {
			# found and stripped @rus
			s/~~/~/g;
			s/^\.//;
			$msgdb{$_} = {};
#			print "Saved one-line message: '$_'\n";
			next;
		}

#		print "Found new message start: '$_'\n";
		$msg = $_ . "\\n";
		$search_rus = 1;
		next;
	}
}
close(IN);

#my @msgdb_eng = keys %msgdb;
#$" = "\n";
#print "All english msgdb messages:\n";
#print "@msgdb_eng\n";

#
# process input files
foreach my $i (@ARGV) {
	process_file($i);
}

print "===> Unused messages:\n";
foreach my $i (keys %msgdb) {
	next if (%{$msgdb{$i}});
	print "\t'$i'\n";
}
print "\n";

dump_messages("===> Messages used with '\\n' added", \%msgdb_nl);
dump_messages("===> Messages used with '\\n' stripped", \%msgdb_nonl);
dump_messages("===> Messages without translation", \%msgdb_notfound);
