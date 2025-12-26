package cmdl;
use warnings;
use strict;
use Class::Struct;
struct( 'cmdl', {
    input_color => '$',
    enabled => '$',
    cursor => '$',
    input => '$',
    rb => '$', # Render buffer index.
    rb_input_node => '$',
});

sub new {
    $cmdl::enabled = 0;
    $cmdl::cursor = 0;
    $cmdl::input = "";

    $cmdl::input_color = 0xFFFFFF;
    
    my $num_rb_nodes = 16;
    $cmdl::rb = Nemi::new_renderbuf($num_rb_nodes);

    Nemi::rb_use_cellcoords($cmdl::rb);
    $cmdl::rb_input_node = Nemi::rb_add_text($cmdl::rb, 0, 0, $cmdl::input, $cmdl::input_color);
     
    Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_input_node);
}


sub update_view {
    Nemi::rb_update_text($cmdl::rb, $cmdl::rb_input_node, 
        2,
        Nemi::term_get_rows()-1, 
        "> ".$cmdl::input, $cmdl::input_color);
}
sub toggle_enabled {
    $cmdl::enabled = !$cmdl::enabled;
    print("Enabled? $cmdl::enabled\n");
    if($cmdl::enabled) {
        Nemi::term_ignore_chars();
        Nemi::term_ignore_keys();
        Nemi::rb_show_node($cmdl::rb, $cmdl::rb_input_node);
        update_view();
    }
    else {
        Nemi::term_unignore_chars();
        Nemi::term_unignore_keys();
        Nemi::rb_hide_node($cmdl::rb, $cmdl::rb_input_node);
    }
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
    
        $cmdl::input = "";
        $cmdl::cursor = 0;
        cmdl->update_view();
    }
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


