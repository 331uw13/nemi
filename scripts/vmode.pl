use warnings;
use strict;

# NOTE: global variables must be set from 'init_script'



sub event_char_input {
    print("(vmode) - $_[0]\n");

    if(Nemi::keydown(32) == 1) {
        print("  > space <\n");
    }
}

sub event_key_input {
    #print("(vmode) - $_[0]\n");
}


sub init_script {
    my $term_rows = Nemi::get_terminal_rows();
    my $term_cols = Nemi::get_terminal_cols();
    print("(vmode) Terminal rows = $term_rows\n");
    print("(vmode) Terminal cols = $term_cols\n");
}


