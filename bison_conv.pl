print '#include "stdafx.h"',"\n";
print '#include "yy.h"',"\n";

$flag = 0;

while ( <> ) {
    chop;
	if ( /= yylex/ ){
	    s/= yylex/= c_compiler::parse::identifier::judge_impl::prev = yylex/;
		++$flag;
	}
	if ( /= YYLEX/ ){
	    s/= YYLEX/= c_compiler::parse::identifier::judge_impl::prev = YYLEX/;
		++$flag;
	}
	if ( /c.tab.c/ ){
		s/c.tab.c/c_y.cpp/g;
	}
    print $_, "\n";
}

if ( $flag != 1 ) {
	print STDERR "Error detected at $0\n";
	print STDERR '$flag = ', $flag, "\n";
	exit 1;
}
