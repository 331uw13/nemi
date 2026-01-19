use strict;
use warnings;
use feature qw(switch);


our $script_name;
our $cmdl_enabled;
our $cmdl_input;
our $cursor_x;

our $cmdl_prefix;
our $cursor_color;
our $input_fg_color;
our $input_bg_color;

# Ideas:
# Auto complete
# Command to show all functions.


sub execute_cmd {
    my $retval = eval($cmdl_input);
    if($@) {
        chomp($@);
        nemi::create_msg("\033[31mFailed to run command '$cmdl_input'\033[0m");
        nemi::create_msg("\033[31m$@\033[0m");
        return;
    }

    if(!($cmdl_input =~ m/^(nemi::)/s)) {
        nemi::create_msg("$retval");
    }
}

#!REGISTER_EVENT
sub event_render {
    if(!$cmdl_enabled) {
        return;
    }

    my $draw_row = nemi::term_get_rows() - 1;
    my $prefix_len = length($cmdl_prefix);
    
    nemi::draw_rect_cells(-1, $draw_row, nemi::term_get_cols()+2, 1, $input_bg_color);

    nemi::draw_rect_cells($cursor_x + $prefix_len, $draw_row, 1, 1, $cursor_color);
    nemi::draw_text_cells(0, $draw_row, $cmdl_prefix . $cmdl_input, $input_fg_color);
}

#!REGISTER_EVENT
sub event_char_input {
    if(!$cmdl_enabled) {
        return;
    }

    substr($cmdl_input, $cursor_x, 0) = chr($_[0]);
    $cursor_x++;
}

#!REGISTER_EVENT
sub event_keybind_press {
    my $event_name = $_[0];

    if($event_name eq "toggle") {
        if(($cmdl_enabled = !$cmdl_enabled)) {
            nemi::term_ignore_chars();
            nemi::term_ignore_keys();
            nemi::term_hide_cells(0, nemi::term_get_rows()-1, nemi::term_get_cols(), 1);
        }
        else {
            nemi::term_unignore_chars();
            nemi::term_unignore_keys();
            nemi::term_show_cells(0, nemi::term_get_rows()-1, nemi::term_get_cols(), 1);
        }
        return;
    }

    if(!$cmdl_enabled) {
        return;
    }


    given($event_name) {
        when("erase_char") {
            if($cursor_x > 0) {
                substr($cmdl_input, $cursor_x-1, 1, '');
                $cursor_x--;
            }
        }
        when("move_cursor_left") {
            if($cursor_x > 0) {
                $cursor_x--;
            }
        }
        when("move_cursor_right") {
            if($cursor_x+1 <= length($cmdl_input)) {
                $cursor_x++;
            }
        }
        when("execute_cmd") {
            execute_cmd();
            $cmdl_input = "";
            $cursor_x = 0;
        }
    }

}

sub init_script {
    $script_name = "cmdl";
    $cmdl_input = "";
    $cmdl_prefix = "> ";
    $cmdl_enabled = 0;
    $cursor_x = 0;

    $cursor_color = 0x50AE00;
    $input_fg_color = 0xFFFFFF;
    $input_bg_color = 0x171b23;

    nemi::add_keybind($script_name, "toggle", "lctrl + 0");
    nemi::add_keybind($script_name, "erase_char", "backspace");
    nemi::add_keybind($script_name, "move_cursor_left", "left");
    nemi::add_keybind($script_name, "move_cursor_right", "right");
    nemi::add_keybind($script_name, "execute_cmd", "enter");

    
}

#!REGISTER_EVENT
sub event_help_message {
    nemi::create_msg(
        "=== cmdl.pl help ===\n\r".
        "--------------------\n\r".
        " You can evaluate perl expressions,\n\r".
        " the output is going to be written here.\n\r".
        " for example you can try to write 2 * 1024 to the command line.\n\r".
        "\n".
        " You can also call functions to C code side\n\r".
        " but this way some of them may behave unexpectedly.\n\r".
        " Its better to write a script if you need to add specific behaviour.\n\r".
        " For script development you can follow instructions from:\n\r".
        "   https://github.com/331uw13/nemi/tree/main/scripts\n\r".
        "   or read the README.md from the repo.\n\r".
        "\n"
    );
}
