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

sub get_modules {
	my $i = 0;
	my @modules;

	for $_ (@_) {
		next if (/sog/);
		next if (/make-module-depend/);

		my $dir = $_;
		$modules[$i++] = { 'dir' => $dir, 'module' => mod_name($dir) };
	}

	return @modules;
}

1;
