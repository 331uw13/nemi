use warnings;
use strict;
use feature qw(switch);

our $script_name;



#!REGISTER_EVENT
sub event_keybind_press {
    my $event_name = $_[0];
    given($event_name) {
        when("scroll_up") {
            nemi::term_yscroll(-1);
        }
        when("scroll_down") {
            nemi::term_yscroll(1);
        }
    }
}

sub init_script {
    $script_name = "scrollback";
    nemi::add_keybind($script_name, "scroll_up", "lshift + lctrl + i");
    nemi::add_keybind($script_name, "scroll_down", "lshift + lctrl + k");
}


#!REGISTER_EVENT
sub event_help_message {
    nemi::create_msg(
        "=== $script_name.pl help ===\n\r".
        " Allows user to scroll the terminal buffer\n\r".
        " with keybinds.\n\r".
        " Run 'Nemi::script_keybinds(\"$script_name\")' in command line\n\r".
        " to see keybinds.\n\r"
    );
}

