while (<>) {
    chop;
    if (/\" = ([0-9]+)/) {
	print $1, "\n";
    }
}
