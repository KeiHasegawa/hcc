$dq = shift @ARGV;

while (<>) {
    chop;
    next if (/DFA Dump:/);
    goto label;
}

label:
while (<>) {
    chop;
    if (/state # ([0-9]+):/) {
	$state = $1;
	$_ = <>;
	chop;
	/	([0-9]+)	([0-9]+)/;
	if ($1 == 1) {
	    if ($state == $2) {
		while (<>) {
		    chop;
		    if (/	([0-9]+)	([0-9]+)/) {
			$trans = $2;
			if ($1 == $dq) {
			    if ($trans != $state) {
				$xxx[$cnt] = $state;
				++$cnt;
			    }
			    goto label2;
			}
		    }
		}
	    }
	}
      label2:
    }
}

print<<EOF
  static int residual;
  if (!residual)
    yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
  else {
    yy_current_state = residual;
    residual = 0;
  }
  if (
EOF
    ;

for ($i = 0 ; $i != $cnt ; ++$i) {
    if ($i != 0) {
      print " || ";
    }
    print "yy_current_state == ", $xxx[$i];
}

print<<EOF2
  ) {
    if (yy_cp[0] < 0 && yy_cp[1] == '\\\\') {
      residual = yy_current_state;
    }
  }
EOF2

