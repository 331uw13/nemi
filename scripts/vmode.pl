
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
our $test_rect;
our $test_text;


sub event_char_input {}
sub event_key_input {
    
    # LEFT_CTRL + M
    if(Nemi::keydown(341) && Nemi::keydown(77)) {
        vmode::toggle_enable();
        print("vmode enabled : $vmode::enabled\n");
    }

    #if($vmode::enabled) {
    #    Nemi::rb_update_rect($renderbuf, $test_rect, 300, 200, 50, 50, 0xFF30FF);
    #}
    #else {
    #    Nemi::rb_update_rect($renderbuf, $test_rect, 50, 200, 100, 100, 0x25FA32);
    #}
}


sub init_script {

    # No "fast phase rendering" is happening from perl script.
    # it just updates the render buffer when needed.

    # Script's render buffer will get freed automatically
    # if it was created when the script is unloaded.
    my $renderbuf_max_meshes = 80;
    $renderbuf = Nemi::new_renderbuf($renderbuf_max_meshes);

    #Nemi::rb_use_arbcoords(); # Arbitrary coordinates. (X, Y)
    #Nemi::rb_use_cellcoords(); # Cell coordinates. (Row, Column)


    my $test_rect_0 = Nemi::rb_add_rect($renderbuf, 100, 100, 50, 50, 0x30FF30);
    #my $test_rect_1 = Nemi::rb_add_rect($renderbuf, 100, 100, 30, 30, 0xFFFFFF);
    #my $test_rect_2 = Nemi::rb_add_rect($renderbuf, 100, 100, 30, 30, 0xFFFFFF);
    #my $test_rect_3 = Nemi::rb_add_rect($renderbuf, 100, 100, 30, 30, 0xFFFFFF);
    #$test_text = Nemi::rb_add_text($renderbuf, 80, 300, "Hello text from perl", 0xFFFFFF);
    



    $vmode = vmode->new();

    my $term_rows = Nemi::term_get_rows();
    my $term_cols = Nemi::term_get_cols();
    print("(vmode) Terminal rows = $term_rows\n");
    print("(vmode) Terminal cols = $term_cols\n");
}


