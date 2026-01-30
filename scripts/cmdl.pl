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
our @ext_funcs;
our @cmd_suggestions;
our $cmd_suggestion_cursor;


sub execute_cmd {
    if(length($cmdl_input) == 0) {
        return;
    }

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



    my $draw_x = $prefix_len;
    #foreach my $suggestion (@cmd_suggestions) {
    for(my $i = $cmd_suggestion_cursor; $i < scalar(@cmd_suggestions); $i++) {
        my $suggestion = $cmd_suggestions[$i];


        nemi::draw_text_cells($draw_x, $draw_row-1, $suggestion, 
            ($i eq $cmd_suggestion_cursor) ? 0x3f724d : 0x3a3a3a
        );

        $draw_x += length($suggestion) + 1;
    }

    nemi::draw_rect_cells($cursor_x + $prefix_len, $draw_row, 1, 1, $cursor_color);
    nemi::draw_text_cells(0, $draw_row, $cmdl_prefix . $cmdl_input, $input_fg_color);
}

sub find_cmd_suggestions {
    @cmd_suggestions = ( );

    foreach my $func (@ext_funcs) {
        if(index($func, $cmdl_input) >= 0) {
            push(@cmd_suggestions, $func);
        }
    }

    my $cmd_suggestions_len = scalar(@cmd_suggestions);
    if($cmd_suggestion_cursor >= $cmd_suggestions_len) {
        $cmd_suggestion_cursor = $cmd_suggestions_len - 1;
    }
    if($cmd_suggestion_cursor < 0) {
        $cmd_suggestion_cursor = 0;
    }
}

#!REGISTER_EVENT
sub event_char_input {
    if(!$cmdl_enabled) {
        return;
    }

    substr($cmdl_input, $cursor_x, 0) = chr($_[0]);
    find_cmd_suggestions();
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
                find_cmd_suggestions();
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
        when("pick_suggestion") {
            $cmdl_input = $cmd_suggestions[$cmd_suggestion_cursor];
            $cursor_x = length($cmdl_input);
            find_cmd_suggestions();
        }
        when("execute_cmd") {
            execute_cmd();
            @cmd_suggestions = ( );
            $cmdl_input = "";
            $cursor_x = 0;
        }
        when("cycle_cmd_suggestions_right") {
            if(scalar(@cmd_suggestions) > 0) {
                $cmd_suggestion_cursor++;
                $cmd_suggestion_cursor %= scalar(@cmd_suggestions);
            }
        }
        when("cycle_cmd_suggestions_left") {
            if(scalar(@cmd_suggestions) > 0) {
                $cmd_suggestion_cursor--;
                $cmd_suggestion_cursor %= scalar(@cmd_suggestions);
            }
        }
    }

}

sub init_script {
    $script_name = "cmdl";
    $cmdl_input = "";
    $cmdl_prefix = "> ";
    $cmdl_enabled = 0;
    $cursor_x = 0;
    @cmd_suggestions = ( );
    $cmd_suggestion_cursor = 0;
    $cursor_color = 0x50AE00;
    $input_fg_color = 0xFFFFFF;
    $input_bg_color = 0x171b23;

    nemi::add_keybind($script_name, "toggle", "lctrl + 0");
    nemi::add_keybind($script_name, "erase_char", "backspace");
    nemi::add_keybind($script_name, "move_cursor_left", "left");
    nemi::add_keybind($script_name, "move_cursor_right", "right");
    nemi::add_keybind($script_name, "execute_cmd", "enter");
    nemi::add_keybind($script_name, "cycle_cmd_suggestions_left", "lctrl + l");
    nemi::add_keybind($script_name, "cycle_cmd_suggestions_left", "lctrl + left");
    nemi::add_keybind($script_name, "cycle_cmd_suggestions_right", "lctrl + j");
    nemi::add_keybind($script_name, "cycle_cmd_suggestions_right", "lctrl + right");
    nemi::add_keybind($script_name, "pick_suggestion", "tab");
    foreach my $entry (keys %nemi::) {
        no strict 'refs';
        if (defined &{"nemi::$entry"}) {
            push(@ext_funcs, "nemi::$entry");
        }
    }
}

#!REGISTER_EVENT
sub event_help_message {
    nemi::create_msg(
        "=== $script_name.pl help ===\n\r".
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
