sub mod_name {
	my $dir = shift;
	my $filename = $dir . "/Makefile";

	open(IN, $filename) || die "$!: can't open $filename";
	while (<IN>) {
		chomp;
		if (/MODULE/) {
			s/MODULE//;
			s/[ \t=]*//;
			return $_;
		}
	}

	die "$filename: can't get module name"
}

1;
