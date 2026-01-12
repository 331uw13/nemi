package vmode;
use warnings;
use strict;
use Class::Struct;

struct( 'vmode', {
    enabled => '$',
    cursor_x => '$',
    cursor_y => '$',
    select_start_x => '$',
    select_start_y => '$',
    select_mode => 0,
    rb => '$', # Render buffer.
    rb_cursor_node => '$',
    cursor_color => '$',
    select_color_bg => '$',
    select_color_fg => '$',
    word_separators => '@'
});

sub init {
    $vmode::word_separators = ( 0, ' ' );
    $vmode::enabled = 0;
    $vmode::cursor_color = 0xA03366;
    $vmode::select_color_bg = 0x6D2446;
    $vmode::select_color_fg = 0xDD9FBC;
    $vmode::rb = Nemi::new_renderbuf(8);
    Nemi::rb_use_cellcoords($vmode::rb);
    $vmode::rb_cursor_node = Nemi::rb_add_rect($vmode::rb, 0, 0, 1, 1, $vmode::cursor_color);
    $vmode::cursor_x = 0;
    $vmode::cursor_y = 0;
    $vmode::select_start_x = 0;
    $vmode::select_start_y = 0;
    
    Nemi::rb_hide_node($vmode::rb, $vmode::rb_cursor_node);
}

package main;
use warnings;
use strict;
use feature qw(switch);

our $script_name;
our $vmode;

sub min {
    return $_[0] < $_[1] ? $_[0] : $_[1];
}

sub max {
    return $_[0] > $_[1] ? $_[0] : $_[1];
}


# Block select
#sub show_selected_cells {
#    my $start_x = min($vmode::cursor_x, $vmode::select_start_x);
#    my $start_y = min($vmode::cursor_y, $vmode::select_start_y);
#    my $end_x = max($vmode::cursor_x, $vmode::select_start_x);
#    my $end_y = max($vmode::cursor_y, $vmode::select_start_y);
#    $end_y++;
#
#    for(my $y = $start_y; $y < $end_y; $y++) {
#        for(my $x = $start_x; $x < $end_x; $x++) {
#            if($_[0]) {
#                Nemi::term_set_cell_custom_bg($x, $y, $vmode::select_color_bg); 
#                Nemi::term_set_cell_custom_fg($x, $y, $vmode::select_color_fg); 
#            }
#            else {
#                Nemi::term_clear_cell_custom_bg($x, $y); 
#                Nemi::term_clear_cell_custom_fg($x, $y); 
#            }
#        }
#    }
#}

sub select_cell {
    my $x = $_[1];
    my $y = $_[2];
    if($_[0]) {
        Nemi::term_set_cell_custom_bg($x, $y, $vmode::select_color_bg); 
        Nemi::term_set_cell_custom_fg($x, $y, $vmode::select_color_fg); 
    }
    else {
        Nemi::term_clear_cell_custom_bg($x, $y); 
        Nemi::term_clear_cell_custom_fg($x, $y); 
    }
}

sub show_selected_cells {
    my $is_shown = $_[0];
        
    my $start_y = min($vmode::cursor_y, $vmode::select_start_y);
    my $end_y = max($vmode::cursor_y, $vmode::select_start_y);


    if($start_y == $end_y) { # Single line selected.
        my $start_x = min($vmode::cursor_x, $vmode::select_start_x);
        my $end_x = max($vmode::cursor_x, $vmode::select_start_x);

        for(my $x = $start_x; $x < $end_x; $x++) {
            select_cell($is_shown, $x, $start_y);
        }
    }
    else {  # Multiple lines selected.

        $end_y++;
        my $end_x = $vmode::cursor_x;#min($vmode::cursor_x, $vmode::select_start_x);
        my $start_x = $vmode::select_start_x;#max($vmode::cursor_x, $vmode::select_start_x);

        if($vmode::cursor_y < $vmode::select_start_y) {
            $start_x = $vmode::cursor_x;
            $end_x = $vmode::select_start_x; 
        }

        my $x = $start_x;
        my $y = $start_y;
        my $term_cols = Nemi::term_get_cols();

        for(my $y = $start_y; $y < $end_y; $y++) {
            my $x_ln_stop = $y+1 >= $end_y ? $end_x : $term_cols;
            for(my $x = $start_x; $x < $x_ln_stop; $x++) {
                select_cell($is_shown, $x, $y);
            }
            $start_x = 0;
        }
    }
}


sub toggle_select_mode {
    $vmode::select_mode = !$vmode::select_mode;
    if($vmode::select_mode) {
        print("Select mode enabled.\n");
    }
    else {
        print("Select mode disabled.\n");
        show_selected_cells(0);
    }

    $vmode::select_start_x = $vmode::cursor_x;
    $vmode::select_start_y = $vmode::cursor_y;
}

sub move_cursor {
    if($vmode::select_mode) {
        show_selected_cells(0);
    }

    $vmode::cursor_x += int($_[0]);
    $vmode::cursor_y += int($_[1]);

    if($vmode::select_mode) {
        show_selected_cells(1);
    }
}

sub update_view {
    Nemi::rb_update_rect($vmode::rb, $vmode::rb_cursor_node, $vmode::cursor_x, $vmode::cursor_y, 1, 1, $vmode::cursor_color);
}


sub toggle_vmode {
    $vmode::enabled = !$vmode::enabled;

    if($vmode::enabled) {
        $vmode::cursor_x = Nemi::term_get_cursor_x()+1;
        $vmode::cursor_y = Nemi::term_get_cursor_y();
        Nemi::term_ignore_chars();
        Nemi::term_ignore_keys();
        Nemi::rb_show_node($vmode::rb, $vmode::rb_cursor_node);
    }
    else {
        Nemi::term_unignore_chars();
        Nemi::term_unignore_keys();
        Nemi::rb_hide_node($vmode::rb, $vmode::rb_cursor_node);
    
        if($vmode::select_mode) {
            show_selected_cells(0);
        }
        $vmode::select_mode = 0;
    }
            
    update_view();
}


sub get_word_jump {
    my $start_x = $_[0];
    my $end_x = $_[1];

    if($start_x == $end_x) {
        return 0;
    }
    my $direction = $start_x > $end_x ? -1 : +1;
    my $x = $start_x;

    while(1) {
        my $char      = Nemi::term_get_char($x, $vmode::cursor_y);
        my $next_char = Nemi::term_get_char($x + $direction, $vmode::cursor_y);
        if($char eq 0 and $next_char eq 0) {
            last;
        }
        
        $x += $direction;

        if(chr($char) ~~ $vmode::word_separators and not chr($next_char) ~~ $vmode::word_separators) {
            last;
        }

        if($direction < 0 and $x <= $end_x) {
            last;
        }
        if($direction > 0 and $x >= $end_x) {
            last;
        }
    }

    return $x - $start_x;
}


#!REGISTER_EVENT
sub event_keybind_press {
    my $event_name = $_[0];
    if($event_name eq "toggle") {
        toggle_vmode();
        return;
    }

    if(!$vmode::enabled) {
        return;
    }

    my $do_update = 0;
    
    given($event_name) {
        when("move_cursor_up") {
            move_cursor(0, -1);
            $do_update = 1;
        }
        when("move_cursor_down") {
            move_cursor(0, 1);
            $do_update = 1;
        }
        when("move_cursor_left") {
            move_cursor(-1, 0);
            $do_update = 1;
        }
        when("move_cursor_right") {
            move_cursor(1, 0);
            $do_update = 1;
        }
        when("word_jump_right") {
            move_cursor(get_word_jump($vmode::cursor_x, Nemi::term_get_cols()), 0);
            $do_update = 1;
        }
        when("word_jump_left") {
            move_cursor(get_word_jump($vmode::cursor_x, 0), 0);
            $do_update = 1;
        }
        when("toggle_select_mode") {
            toggle_select_mode(); 
            $do_update = 1;
        }
        when("copy_selected") {
            if($vmode::select_mode) {
                Nemi::term_copy_to_clipboard(
                    $vmode::select_start_x,
                    $vmode::select_start_y,
                    $vmode::cursor_x,
                    $vmode::cursor_y
                );
            }
        }
    }

    if($do_update) {
        update_view();
    }
}

sub init_script {
    $script_name = "vmode";
    $vmode = vmode->init();

    Nemi::add_keybind($script_name, "toggle", "lctrl + k");
    Nemi::add_keybind($script_name, "move_cursor_up", "up");
    Nemi::add_keybind($script_name, "move_cursor_up", "i");
    Nemi::add_keybind($script_name, "move_cursor_down", "down");
    Nemi::add_keybind($script_name, "move_cursor_down", "k");
    Nemi::add_keybind($script_name, "move_cursor_left", "left");
    Nemi::add_keybind($script_name, "move_cursor_left", "j");
    Nemi::add_keybind($script_name, "move_cursor_right", "right");
    Nemi::add_keybind($script_name, "move_cursor_right", "l");
    Nemi::add_keybind($script_name, "word_jump_left", "lshift + j");
    Nemi::add_keybind($script_name, "word_jump_right", "lshift + l");
    Nemi::add_keybind($script_name, "toggle_select_mode", "s");
    Nemi::add_keybind($script_name, "copy_selected", "c");

}


#!REGISTER_EVENT
sub event_help_message {
    Nemi::create_msg(
        "=== vmode.pl help ===\n\r".
        " VMode or \"Visual mode\" creates another cursor\n\r" .
        " which the user can move around to any cell position on screen.\n\r" .
        " You can also select and copy text.\n\r" .
        " ... More features will be added later :)\n"
    );
}

