while ( <> ){
    chop $_;
    if ( /c.y:([0-9]+)\.[0-9]+-[0-9]+:(.*)/ ){
	print STDERR "c.y(",$1,") :",$2, "\n";
	next;
    }
    if ( /c.y:([0-9]+)\.[0-9]+:(.*)/ ){
	print STDERR "c.y(",$1,") :",$2, "\n";
	next;
    }
    print STDERR $_, "\n";
}
