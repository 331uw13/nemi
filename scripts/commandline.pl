package cmdl;
use warnings;
use strict;
use Class::Struct;
struct( 'cmdl', {
    input_color => '$',
    cursor_color => '$',
    bgrect_color => '$',
    suggest_color => '$',
    enabled => '$',
    cursor => '$',
    input => '$',
    suggest => '$',
    rb => '$', # Render buffer index.
    rb_input_node => '$',
    rb_bgrect_node => '$',
    rb_cursor_node => '$',
    rb_suggest_node => '$',
    avail_extfuncs => '@'
});

sub init {
    $cmdl::enabled = 0;
    $cmdl::cursor = 0;
    $cmdl::input = "";
    $cmdl::suggest = "";

    $cmdl::input_color = 0xFFFFFF;
    $cmdl::bgrect_color = 0x171b23;
    $cmdl::cursor_color = 0x50AE00;
    $cmdl::suggest_color = 0x206270;

    my $num_rb_nodes = 8;
    $cmdl::rb = Nemi::new_renderbuf($num_rb_nodes);

    Nemi::rb_use_cellcoords($cmdl::rb);

    $cmdl::rb_bgrect_node = Nemi::rb_add_rect($cmdl::rb, 0, 0, 0, 0, $cmdl::bgrect_color);
    $cmdl::rb_cursor_node = Nemi::rb_add_rect($cmdl::rb, 0, 0, 0, 0, $cmdl::cursor_color);
    $cmdl::rb_input_node = Nemi::rb_add_text($cmdl::rb, 0, 0, "", $cmdl::input_color);
    $cmdl::rb_suggest_node = Nemi::rb_add_text($cmdl::rb, 0, 0, "", $cmdl::suggest_color);

    Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_input_node);
    Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_bgrect_node);
    Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_cursor_node);
    Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_suggest_node);
}


sub get_best_cmd_suggestion {
    $cmdl::suggest = "";
    if(length($cmdl::input) == 0) {
        Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_suggest_node);
        return;
    }

    my $match_count = 0;

    my $suffix = " | ";
    foreach my $fn (@cmdl::avail_extfuncs) {
        if(rindex($fn, $cmdl::input, 0) != -1) {
            $cmdl::suggest .= $fn . $suffix;
            $match_count++;

            if($match_count > 4) {
                last;
            }
        }
    }
    
    if($match_count == 0) {
        Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_suggest_node);
    }
    else {
        $cmdl::suggest = substr($cmdl::suggest, 0, length($cmdl::suggest) - length($suffix));
        Nemi::rb_show_node($cmdl::rb, $cmdl::rb_suggest_node);
    }
}

sub update_view {

    my $x = 1;
    my $y = Nemi::term_get_rows() - 1;

    # Input text.
    my $prefix = "> ";
    Nemi::rb_update_text($cmdl::rb, $cmdl::rb_input_node, 
        $x, $y, 
        $prefix . $cmdl::input, $cmdl::input_color
    );

    # Background rect.
    Nemi::rb_update_rect($cmdl::rb, $cmdl::rb_bgrect_node, 
        0, $y,
        Nemi::term_get_cols(), 1, $cmdl::bgrect_color);

    # Cursor rect.
    Nemi::rb_update_rect($cmdl::rb, $cmdl::rb_cursor_node,
        $x + $cmdl::cursor + length($prefix),
        $y,
        1, 1, $cmdl::cursor_color);


    get_best_cmd_suggestion();

    my $suggest_len = length($cmdl::suggest);
    if($suggest_len == 0) {
        return;
    }
    my $input_len = length($cmdl::input);
    if($input_len <= $suggest_len) {
        my $suggest_text = substr($cmdl::suggest, $input_len, $suggest_len);
        Nemi::rb_update_text($cmdl::rb, $cmdl::rb_suggest_node, 
            $x + length($prefix) + $input_len,
            $y, 
            $suggest_text, $cmdl::suggest_color
        );
    }


}


sub toggle_enabled {
    $cmdl::enabled = !$cmdl::enabled;
    if($cmdl::enabled) {
        Nemi::term_ignore_chars();
        Nemi::term_ignore_keys();
        Nemi::rb_show_node($cmdl::rb, $cmdl::rb_input_node);
        Nemi::rb_show_node($cmdl::rb, $cmdl::rb_bgrect_node);
        Nemi::rb_show_node($cmdl::rb, $cmdl::rb_cursor_node);
        Nemi::rb_show_node($cmdl::rb, $cmdl::rb_suggest_node);
        update_view();
    
        Nemi::term_hide_cells(0, Nemi::term_get_rows()-1, Nemi::term_get_cols(), 1);
    }
    else {
        Nemi::term_unignore_chars();
        Nemi::term_unignore_keys();
        Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_input_node);
        Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_bgrect_node);
        Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_cursor_node);
        Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_suggest_node);
        
        Nemi::term_show_cells(0, Nemi::term_get_rows()-1, Nemi::term_get_cols(), 1);
    }
}


package main;
use warnings;
use strict;
use feature qw(switch);

our $cmdl; 
our $script_name;

sub execute {        
    print("Command input: '$cmdl::input'\n");    

    my $retval = eval($cmdl::input);
    if($@) {
        chomp($@);
        Nemi::create_msg("\033[31mFailed to run command '$cmdl::input'\033[0m");
        Nemi::create_msg("\033[31m$@\033[0m");
        return;
    }

    if(!($cmdl::input =~ m/^(Nemi::)/s)) {
        Nemi::create_msg("$retval");
    }
}

#!REGISTER_EVENT
sub event_keybind_press {
    my $event_name = $_[0];
    
    if($event_name eq "toggle") {
        cmdl->toggle_enabled();
        return;
    }

    if(!$cmdl::enabled) {
        return;
    }

    given($event_name) {
        when("execute_command") {
            execute();
            $cmdl::input = "";
            $cmdl::cursor = 0;
            cmdl->update_view();
        }
        when("erase_char") {
            if($cmdl::cursor > 0) {
                substr($cmdl::input, $cmdl::cursor-1, 1, '');
                $cmdl::cursor--;
                cmdl->update_view();
            }
        }
        when("move_cursor_left") {
            if($cmdl::cursor > 0) {
                $cmdl::cursor--;
                cmdl->update_view();
            }
        }
        when("move_cursor_right") {
            if($cmdl::cursor+1 <= length($cmdl::input)) {
                $cmdl::cursor++;
                cmdl->update_view();
            }
        }
        when("auto_complete") {
            if(length($cmdl::suggest) > 0) {
                my @test = split(/\|/, $cmdl::suggest);

                $cmdl::input = $test[0];
                $cmdl::input =~ tr/ //ds;
                $cmdl::cursor = length($cmdl::input);
                cmdl->update_view();
            }
        }
    }
}

#!REGISTER_EVENT
sub event_win_resized {
    cmdl->update_view();
}

#!REGISTER_EVENT
sub event_char_input {
    if(!$cmdl::enabled) {
        return;
    }

    my $char = chr($_[0]);
   
    substr($cmdl::input, $cmdl::cursor, 0) = $char;
    $cmdl::cursor++;
    cmdl->update_view();
}


sub init_script {
    $script_name = "commandline";
    $cmdl = cmdl->init();

    Nemi::add_keybind($script_name, "toggle", "lctrl + 0");
    Nemi::add_keybind($script_name, "erase_char", "backspace");
    Nemi::add_keybind($script_name, "move_cursor_left", "left");
    Nemi::add_keybind($script_name, "move_cursor_right", "right");
    Nemi::add_keybind($script_name, "auto_complete", "tab");
    Nemi::add_keybind($script_name, "execute_command", "enter");

    # Get available external functions for auto complete.
    foreach my $entry ( keys %Nemi:: ) {
        no strict 'refs';
        if (defined &{"Nemi::$entry"}) {
            push(@cmdl::avail_extfuncs, "Nemi::".$entry."()");
            #print "sub $entry is defined\n" ;
        }
    }
}

#!REGISTER_EVENT
sub event_help_message {
    Nemi::create_msg(
        "=== commandline.pl help ===\n\r".
        "---------------------------\n\r".
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

