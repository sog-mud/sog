sub mod_name {
	my $subdir = shift;
	my $dir = shift;
	my $filename = $subdir . $dir . "/Makefile";

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

sub get_modules {
	my $i = 0;
	my $subdir = shift;
	my $dir;
	my @modules;

	for $dir (@_) {
		$modules[$i++] = {
			'dir' => $dir,
			'module' => mod_name($subdir, $dir)
		};
	}

	return @modules;
}

1;
