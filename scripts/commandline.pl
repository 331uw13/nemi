package cmdl;
use warnings;
use strict;
use Class::Struct;
struct( 'cmdl', {
    input_color => '$',
    cursor_color => '$',
    bgrect_color => '$',
    enabled => '$',
    cursor => '$',
    input => '$',
    rb => '$', # Render buffer index.
    rb_input_node => '$',
    rb_bgrect_node => '$',
    rb_cursor_node => '$'
});

sub new {
    $cmdl::enabled = 0;
    $cmdl::cursor = 0;
    $cmdl::input = "";

    $cmdl::input_color = 0xFFFFFF;
    $cmdl::bgrect_color = 0x171b23;
    $cmdl::cursor_color = 0x50AE00;
    
    my $num_rb_nodes = 8;
    $cmdl::rb = Nemi::new_renderbuf($num_rb_nodes);

    Nemi::rb_use_cellcoords($cmdl::rb);

    $cmdl::rb_bgrect_node = Nemi::rb_add_rect($cmdl::rb, 0, 0, 0, 0, $cmdl::bgrect_color);
    $cmdl::rb_cursor_node = Nemi::rb_add_rect($cmdl::rb, 0, 0, 0, 0, $cmdl::cursor_color);
    $cmdl::rb_input_node = Nemi::rb_add_text($cmdl::rb, 10, 20, "", $cmdl::input_color);


    print("input_node = $cmdl::rb_input_node\n");
    print("bgrect_node = $cmdl::rb_bgrect_node\n");
    print("cursor_node = $cmdl::rb_cursor_node\n");

    Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_input_node);
    Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_bgrect_node);
    Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_cursor_node);
}


sub update_view {

    my $x = 1;
    my $y = Nemi::term_get_rows() - 1;

    my $prefix = "> ";

    Nemi::rb_update_text($cmdl::rb, $cmdl::rb_input_node, 
        $x, $y, 
        $prefix . $cmdl::input, $cmdl::input_color
    );

    Nemi::rb_update_rect($cmdl::rb, $cmdl::rb_bgrect_node, 
        0, $y,
        Nemi::term_get_cols(), 1, $cmdl::bgrect_color);

    Nemi::rb_update_rect($cmdl::rb, $cmdl::rb_cursor_node,
        $x + $cmdl::cursor + length($prefix),
        $y,
        1, 1, $cmdl::cursor_color);

}


sub toggle_enabled {
    $cmdl::enabled = !$cmdl::enabled;
    if($cmdl::enabled) {
        Nemi::term_ignore_chars();
        Nemi::term_ignore_keys();
        Nemi::rb_show_node($cmdl::rb, $cmdl::rb_input_node);
        Nemi::rb_show_node($cmdl::rb, $cmdl::rb_bgrect_node);
        Nemi::rb_show_node($cmdl::rb, $cmdl::rb_cursor_node);
        update_view();
    
        Nemi::term_hide_cells(0, Nemi::term_get_rows()-1, Nemi::term_get_cols(), 8);
    }
    else {
        Nemi::term_unignore_chars();
        Nemi::term_unignore_keys();
        Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_input_node);
        Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_bgrect_node);
        Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_cursor_node);
        
        Nemi::term_show_cells(0, Nemi::term_get_rows()-1, Nemi::term_get_cols(), 8);
    }
}


package main;
use warnings;
use strict;
use feature qw(switch);

our $cmdl; 


sub execute {        
    print("Command input: '$cmdl::input'\n");    

    my $v = eval($cmdl::input);
    if(defined $v) {
        Nemi::create_msg("$v");
        return;
    }



}


#!REGISTER_EVENT
sub event_key_input {
    # https://www.glfw.org/docs/latest/group__keys.html
    # https://www.glfw.org/docs/latest/group__mods.html
    my $mod_ctrl = 0x0002;
    my $key_dot = 46;
    my $key_enter = 257;
    my $key_backspace = 259;
    my $key_left = 263;
    my $key_right = 262;

    # '.' and Control.
    if($_[0] == $key_dot and $_[1] == $mod_ctrl) {
        cmdl->toggle_enabled();
    }
    if(!$cmdl::enabled) {
        return;
    }
   

    given($_[0]) {
        when($key_enter) {
            execute();
            $cmdl::input = "";
            $cmdl::cursor = 0;
            cmdl->update_view();
        }
        when($key_backspace) {
            if($cmdl::cursor > 0) {
                substr($cmdl::input, $cmdl::cursor-1, 1, '');
                $cmdl::cursor--;
                cmdl->update_view();
            }
        }
        when($key_left) {
            if($cmdl::cursor > 0) {
                $cmdl::cursor--;
                cmdl->update_view();
            }
        }
        when($key_right) {
            if($cmdl::cursor+1 <= length($cmdl::input)) {
                $cmdl::cursor++;
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
    $cmdl = cmdl->new();

}


