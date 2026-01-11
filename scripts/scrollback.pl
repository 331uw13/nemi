use warnings;
use strict;

our $script_name;


#!REGISTER_EVENT
sub event_key_input {
}

#!REGISTER_EVENT
sub event_keybind_press {
    print("$script_name: event_keybind_press(): '$_[0]'\n");
}

sub init_script {
    $script_name = "scrollback";
    Nemi::add_keybind($script_name, "do_something", "lctrl + a");
    Nemi::add_keybind($script_name, "scroll_up", "lctrl + i");
    Nemi::add_keybind($script_name, "scroll_down", "lshift + lctrl + k");
}


#!REGISTER_EVENT
sub event_help_message {
    Nemi::create_msg(
        "=== $script_name.pl help ===\n\r".
        " Allows user to scroll the terminal buffer\n\r".
        " with keybinds.\n\r".
        " Run 'Nemi::script_keybinds(\"$script_name\")' in command line\n\r".
        " to see keybinds.\n\r"
    );
}

