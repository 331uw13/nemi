use strict;
use warnings;
use feature qw(switch);

our $script_name;
our $vmode_enabled;
our @word_separators;

our $cursor_x;
our $cursor_y;
our $select_start_x;
our $select_start_y;
our $select_mode_enabled;
our $block_select_enabled;
our $word_oncursor;
our $word_oncursor_len;
our $word_oncursor_x;
our $word_oncursor_y;

our $cursor_color;
our $select_bg_color;
our $select_fg_color;



sub min {
    return $_[0] < $_[1] ? $_[0] : $_[1];
}

sub max {
    return $_[0] > $_[1] ? $_[0] : $_[1];
}

sub select_cell {
    my $x = $_[1];
    my $y = $_[2];
    if($_[0]) {
        nemi::term_set_cell_custom_bg($x, $y, $select_bg_color);
        nemi::term_set_cell_custom_fg($x, $y, $select_fg_color);
    }
    else {
        nemi::term_clear_cell_custom_bg($x, $y);
        nemi::term_clear_cell_custom_fg($x, $y);
    }
}

sub draw_selected_cells {
    my $is_shown = $_[0];

    if($block_select_enabled) {
        my $start_x = min($cursor_x, $select_start_x);
        my $start_y = min($cursor_y, $select_start_y);
        my $end_x = max($cursor_x, $select_start_x);
        my $end_y = max($cursor_y, $select_start_y);
        $end_y++;

        for(my $y = $start_y; $y < $end_y; $y++) {
            for(my $x = $start_x; $x < $end_x; $x++) {
                select_cell($is_shown, $x, $y);
            }
        }
        return;
    }

    my $start_y = min($cursor_y, $select_start_y);
    my $end_y = max($cursor_y, $select_start_y);


    if($start_y == $end_y) {
        # Single line selected.
        my $start_x = min($cursor_x, $select_start_x);
        my $end_x = max($cursor_x, $select_start_x);

        for(my $x = $start_x; $x < $end_x; $x++) {
            select_cell($is_shown, $x, $start_y);
        }
    }
    else {
        # Multiple lines selected.
        $end_y++;
        my $end_x = $cursor_x;
        my $start_x = $select_start_x;

        if($cursor_y < $select_start_y) {
            $start_x = $cursor_x;
            $end_x = $select_start_x;
        }

        my $x = $start_x;
        my $y = $start_y;
        my $term_cols = nemi::term_get_cols();

        for(my $y = $start_y; $y < $end_y; $y++) {
            my $x_ln_stop = $y+1 >= $end_y ? $end_x : $term_cols;
            for(my $x = $start_x; $x < $x_ln_stop; $x++) {
                select_cell($is_shown, $x, $y);
            }
            $start_x = 0;
        }
    }
}


sub update_word_oncursor_highlight {
    if($word_oncursor_len == 0) {
        return;
    }
    my $begin = $word_oncursor_x;
    my $end   = $word_oncursor_x + $word_oncursor_len;
     if($begin > 0) {
        $begin++;
    }
    for(my $x = $begin; $x < $end; $x++) {
        nemi::term_set_cell_custom_attrs($x, $word_oncursor_y, 0x2);
    }
}

sub disable_word_oncursor_highlight {
    if($word_oncursor_len == 0) {
        return;
    }
    my $begin = $word_oncursor_x;
    my $end   = $word_oncursor_x + $word_oncursor_len;
    if($begin > 0) {
        $begin++;
    }
    for(my $x = $begin; $x < $end; $x++) {
        nemi::term_clear_cell_custom_attrs($x, $word_oncursor_y);
    }
}

sub find_word_oncursor {
    $word_oncursor = "";
    $word_oncursor_len = 0;

    my $line = "";
    my $term_cols = nemi::term_get_cols();
    for(my $i = 0; $i < $term_cols; $i++) {
        my $ch = nemi::term_get_char($i, $cursor_y);
        if($ch eq 0) {
            last;
        }
        $line .= chr($ch);
    }

    my $begin = rindex($line, " ", $cursor_x);
    my $end   = index($line, " ", $cursor_x);  
    if($begin < 0) { $begin = 0; }
    if($end < 0)   { $end = length($line); }

    $word_oncursor_x = $begin;
    $word_oncursor_y = $cursor_y;

    $word_oncursor_len = $end - $begin;
    $word_oncursor = substr($line, $begin, $word_oncursor_len);

    #print("$word_oncursor\n");
}

sub move_cursor {
    if($select_mode_enabled) {    
       draw_selected_cells(0);
    }
    
    $cursor_x += int($_[0]);
    $cursor_y += int($_[1]);

    my $cursor_x_max = nemi::term_get_cols() - 1;
    my $cursor_y_max = nemi::term_get_rows();

    if($cursor_x < 0) { 
        $cursor_x = 0;
    }
    if($cursor_x > $cursor_x_max) {
        $cursor_x = $cursor_x_max;
    }

    # Allow cursor_y to go negative on purpose, 
    # It will get characters from the terminal's scrollback buffer
    
    if($cursor_y > $cursor_y_max) {
        $cursor_y = $cursor_y_max;
    }   


    if($select_mode_enabled) {    
       draw_selected_cells(1);
    }
    else {
        disable_word_oncursor_highlight(); # Disable old highlight.
        find_word_oncursor();
        update_word_oncursor_highlight();
    }
}


sub ischr_word_separator {
    foreach my $ch (@word_separators) {
        if($_[0] eq $ch) {
            return 1;
        }
    }
    return 0;
}

sub get_word_jump_left {
    return 0;
}

sub get_word_jump_to_direction {
    my $direction = $_[0];

    my $term_cols = nemi::term_get_cols();
    my $x = $cursor_x;

    if(nemi::term_get_char($cursor_x, $cursor_y) eq 0) {
        while(1) {
            my $curr_char = nemi::term_get_char($x, $cursor_y);
            if($curr_char ne 0) {
                last;
            }
            
            $x += $direction;
            if(($x <= 0) or ($x >= $term_cols)) {
                last;
            }
        }

        return $x - $cursor_x;
    }

    while(1) {
        my $curr_char = nemi::term_get_char($x, $cursor_y);
        my $next_char = nemi::term_get_char($x + $direction, $cursor_y);

        if($curr_char eq 0 and $next_char eq 0) {
            last;
        }

        if(ischr_word_separator($curr_char) and !ischr_word_separator($next_char)) {
            return $x - $cursor_x + $direction;
        }
        $x += $direction;
    }

    return $x - $cursor_x;
}

#!REGISTER_EVENT
sub event_render {
    if(!$vmode_enabled) {
        return;
    }

    nemi::draw_enable_scroll_offset();
    nemi::draw_rect_cells($cursor_x, $cursor_y, 1, 1, $cursor_color);
    nemi::draw_disable_scroll_offset();


    my $info_text = "";
    if($select_mode_enabled) {
        $info_text .= $block_select_enabled ? "(b-select)" : "(select)";
    }
    $info_text .= " [vmode]";
    my $info_text_x = nemi::term_get_cols() - length($info_text)-1;
    my $info_text_y = 0;
    nemi::draw_text_cells($info_text_x, $info_text_y, $info_text, $cursor_color);

}

#!REGISTER_EVENT
sub event_keybind_press {
    my $event_name = $_[0];

    if($event_name eq "toggle") {
        if(($vmode_enabled = !$vmode_enabled)) {
            nemi::term_ignore_chars();
            nemi::term_ignore_keys();
            $cursor_x = nemi::term_get_cursor_x() + 1;
            $cursor_y = nemi::term_get_cursor_y();
            $select_mode_enabled = 0;
            $block_select_enabled = 0;
        }
        else {
            nemi::term_unignore_chars();
            nemi::term_unignore_keys();
            draw_selected_cells(0);
            disable_word_oncursor_highlight();
        }
        return;
    }

    if(!$vmode_enabled) {
        return;
    }

    given($event_name) {
        when("move_cursor_up") {
            move_cursor(0, -1);
        }
        when("move_cursor_down") {
            move_cursor(0, 1);
        }
        when("move_cursor_left") {
            move_cursor(-1, 0);
        }
        when("move_cursor_right") {
            move_cursor(1, 0);
        }
        when("word_jump_right") {
            move_cursor(get_word_jump_to_direction(+1), 0);
        }
        when("word_jump_left") {
            move_cursor(get_word_jump_to_direction(-1), 0);
        }
        when("toggle_select_mode") {
            if(($select_mode_enabled = !$select_mode_enabled)) {
                $select_start_x = $cursor_x;
                $select_start_y = $cursor_y;
                $word_oncursor = "";
                disable_word_oncursor_highlight();
            }
            else {
                draw_selected_cells(0);
                find_word_oncursor();
                update_word_oncursor_highlight();
            }
        } 
        when("copy_selected") {
            if($select_mode_enabled) {
                nemi::term_copy_to_clipboard(
                    $select_start_x,
                    $select_start_y,
                    $cursor_x,
                    $cursor_y,
                    $block_select_enabled ? "block" : "normal"
                );
            }
        }
        when("toggle_block_select") {
            if($select_mode_enabled) {
                draw_selected_cells(0);
                $block_select_enabled = !$block_select_enabled;
                draw_selected_cells(1);
            }
        }
    }

}


sub init_script {
    $script_name = "vmode";
    $cursor_x = 0;
    $cursor_y = 0;
    $select_start_x = 0;
    $select_start_y = 0;
    $word_oncursor = "";
    $word_oncursor_len = 0;
    $word_oncursor_x = 0;
    $word_oncursor_y = 0;

    $cursor_color = 0xA03366;
    $select_bg_color = 0x6D2446;
    $select_fg_color = 0xDD9FBC;

    # Note: word separators has to be written as integers
    # because 'nemi::term_get_char()' returns int.
    @word_separators = ( 0x0, 0x20 );

    nemi::add_keybind($script_name, "toggle", "lctrl + k");
    nemi::add_keybind($script_name, "move_cursor_up", "up");
    nemi::add_keybind($script_name, "move_cursor_up", "i");
    nemi::add_keybind($script_name, "move_cursor_down", "down");
    nemi::add_keybind($script_name, "move_cursor_down", "k");
    nemi::add_keybind($script_name, "move_cursor_left", "left");
    nemi::add_keybind($script_name, "move_cursor_left", "j");
    nemi::add_keybind($script_name, "move_cursor_right", "right");
    nemi::add_keybind($script_name, "move_cursor_right", "l");
    nemi::add_keybind($script_name, "word_jump_left", "lshift + j");
    nemi::add_keybind($script_name, "word_jump_right", "lshift + l");
    nemi::add_keybind($script_name, "toggle_select_mode", "s");
    nemi::add_keybind($script_name, "copy_selected", "c");
    nemi::add_keybind($script_name, "toggle_block_select", "b");
}


#!REGISTER_EVENT
sub event_help_message {
    nemi::create_msg(
        "=== vmode.pl help ===\n\r".
        " VMode or \"Visual mode\" creates another cursor\n\r" .
        " which the user can move around to any cell position on screen.\n\r" .
        " You can also select and copy text.\n\r" .
        " View files under cursor.\n\r" .
        " Run 'Nemi::script_keybinds(\"$script_name\")' to see all features.\n\r"
    );
}


