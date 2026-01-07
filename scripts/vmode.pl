package vmode;
use warnings;
use strict;
use Class::Struct;
struct( 'vmode', {
    enabled => '$',
    cursor_x => '$',
    cursor_y => '$',
    rb => '$', # Render buffer.
    rb_cursor_node => '$',
    cursor_color => '$'
});

sub init {
    $vmode::enabled = 0;
    $vmode::cursor_color = 0xA03366;
    $vmode::rb = Nemi::new_renderbuf(2);
    Nemi::rb_use_cellcoords($vmode::rb);
    $vmode::rb_cursor_node = Nemi::rb_add_rect($vmode::rb, 0, 0, 1, 1, $vmode::cursor_color);
    $vmode::cursor_x = 0;
    $vmode::cursor_y = 0;
    
    Nemi::rb_hide_node($vmode::rb, $vmode::rb_cursor_node);
}


sub update_view {
    if($vmode::enabled) {
        Nemi::rb_show_node($vmode::rb, $vmode::rb_cursor_node);
    }
    else {
        Nemi::rb_hide_node($vmode::rb, $vmode::rb_cursor_node);
    }
}

sub toggle {
    $vmode::enabled = !$vmode::enabled;
    update_view();

    if($vmode::enabled) {
        Nemi::term_ignore_chars();
        Nemi::term_ignore_keys();
    }
    else {
        Nemi::term_unignore_chars();
        Nemi::term_unignore_keys();
    }
}

package main;
use warnings;
use strict;


our $vmode;



#!REGISTER_EVENT
sub event_key_input {
    # https://www.glfw.org/docs/latest/group__keys.html
    # https://www.glfw.org/docs/latest/group__mods.html
    my $mod_ctrl = 0x0002;
    my $key_n = 78; 
    
    if($_[0] == $key_n and $_[1] == $mod_ctrl) {
        vmode->toggle();
    }

    #if($_[0] == $key_n and $_[1] == $mod_ctrl) {
   
    #    my $term_columns = Nemi::term_get_cols();
    #    for(my $i = 0; $i < $term_columns; $i++) {
    #        my $char = chr(Nemi::term_get_char($i, 0));
    #        print("$char");
    #    }

    #    print("\n");
    #}
}

sub init_script {
    $vmode = vmode->init();
}


