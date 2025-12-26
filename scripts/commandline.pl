package cmdl;
use warnings;
use strict;
use Class::Struct;
struct( 'cmdl', {
    enabled => '$',
    cursor => '$',
    input => '$',
    rb => '$', # Render buffer index.
    input_text => '$',
});

sub new {
    $cmdl::enabled = 0;
    $cmdl::cursor = 0;
    $cmdl::input = "";

    
    my $num_rb_nodes = 16;
    $cmdl::rb = Nemi::new_renderbuf($num_rb_nodes);

    Nemi::rb_use_cellcoords($cmdl::rb);
    $cmdl::input_text = Nemi::rb_add_text($cmdl::rb, 1, 10, "> cmdl", 0xFFFFFF);
        
    Nemi::rb_hide_node($cmdl::rb, $cmdl::input_text);
}

sub toggle_enabled {
    $cmdl::enabled = !$cmdl::enabled;
    print("Enabled? $cmdl::enabled\n");
    if($cmdl::enabled) {
        Nemi::term_ignore_chars();
        Nemi::term_ignore_keys();
        Nemi::rb_show_node($cmdl::rb, $cmdl::input_text);
    }
    else {
        Nemi::term_unignore_chars();
        Nemi::term_unignore_keys();
        Nemi::rb_hide_node($cmdl::rb, $cmdl::input_text);
    }
}

sub update_view {
}

package main;
use warnings;
use strict;


our $cmdl; 


#!REGISTER_EVENT
sub event_key_input {
    # https://www.glfw.org/docs/latest/group__keys.html
    # https://www.glfw.org/docs/latest/group__mods.html
    my $mod_ctrl = 0x0002;
    my $key_dot = 46;
    my $key_enter = 257;

    # '.' and Control.
    if($_[0] == $key_dot and $_[1] == $mod_ctrl) {
        cmdl->toggle_enabled();
    }
    elsif($_[0] == $key_enter) {
        print("Command input: '$cmdl::input'\n");    
    }
}


#!REGISTER_EVENT
sub event_char_input {
    my $char = chr($_[0]);
   
    substr($cmdl::input, $cmdl::cursor, 0) = $char;
    $cmdl::cursor++;
    cmdl->update_view();
}


sub init_script {
    $cmdl = cmdl->new();
}


