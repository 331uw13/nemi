
# NOTE: global variables must be set from 'init_script'

# Links
# https://www.glfw.org/docs/latest/group__keys.html

package vmode;
use warnings;
use strict;
use Class::Struct;
struct( 'vmode', {
    enabled => '$',
    curs_row => '$',
    curs_col => '$'
});

sub new {
    $vmode::enabled = 0;
    $vmode::curs_row = 0;
    $vmode::curs_col = 0;
}

sub toggle_enable {
    $vmode::enabled = !$vmode::enabled;
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

our $renderbuf;


sub event_char_input {}
sub event_key_input {
    
    # LEFT_CTRL + M
    if(Nemi::keydown(341) && Nemi::keydown(77)) {
        vmode::toggle_enable();
        print("vmode enabled : $vmode::enabled\n");
    }
}


sub init_script {

    my $rbuf_max_nodes = 80;
    $renderbuf = Nemi::new_renderbuf($rbuf_max_nodes);
    
    Nemi::rb_use_cellcoords($renderbuf);
    
    my $term_rows = Nemi::term_get_rows();
    my $term_cols = Nemi::term_get_cols();
    print("(vmode) Terminal rows = $term_rows\n");
    print("(vmode) Terminal cols = $term_cols\n");

    #Nemi::rb_use_arbcoords(); # Arbitrary coordinates. (X, Y)
    #Nemi::rb_use_cellcoords(); # Cell coordinates. (Row, Column)


    #$test_rect = Nemi::rb_add_rect($renderbuf, 7, 10, 1, 10, 0x30FFA0);
    #$test_text = Nemi::rb_add_text($renderbuf, 80, 100, "Hello from perl!", 0xFF30FF);
    



    $vmode = vmode->new();

}


