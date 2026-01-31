use strict;
use warnings;
use feature qw(switch);


our $script_name;



#!REGISTER_EVENT
sub event_keybind_press {
    my $event_name = $_[0];

    given($event_name) {
        when("restart_session") {
            nemi::restart();
        }
        when("hotreload_session") {
            nemi::hotreload();
        }
    }
}


sub init_script {
    $script_name = "general";

    nemi::add_keybind($script_name, "restart_session", "f1");
    nemi::add_keybind($script_name, "hotreload_session", "f2");
}


#!REGISTER_EVENT
sub event_help_message {
    nemi::create_msg(
        "=== $script_name.pl help ===\n\r" .
        "----------------------------\n\r" .
        "This script just handles basic keybinds.\n\r" .
        "see 'nemi::script_keybinds(\"$script_name\")' for more info.\n\r"
    );
}

