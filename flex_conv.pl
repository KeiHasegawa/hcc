print '#include "stdafx.h"', "\n";
print '#include "yy.h"', "\n";

$conved = 0;

while (<>) {
    chop;
    if (/yy_current_state = yy_nxt\[yy_base\[yy_current_state\] \+ \(unsigned int\) yy_c\];/) {
	$conved = 1;
	print "#if 1\n";
	print $_, "\n";
	print "#else\n";
	print '#include "flex_patch.p"', "\n";
	print "#endif\n";
    }
    else {
	print $_, "\n";
    }
}

if (!$conved) {
    print STDERR "not converted\n";
    exit 1;
}
