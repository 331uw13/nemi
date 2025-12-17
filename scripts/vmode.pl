
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

sub event_char_input {}
sub event_key_input {
    
    # LEFT_CTRL + M
    if(Nemi::keydown(341) && Nemi::keydown(77)) {
        vmode::toggle_enable();
        print("vmode enabled : $vmode::enabled\n");
    }
}


sub init_script {

    # No "fast phase rendering" is happening from perl script.
    # it just updates the render buffer when needed.

    # Script's render buffer will get freed automatically
    # if it was created when the script is unloaded.
    my $renderbuf_max_meshes = 80;
    my $renderbuf = Nemi::new_renderbuf($renderbuf_max_meshes);

    # When creating a shape, it just creates mesh
    # and the render buffer contains array of meshes.
    # Any add_{shape} function will return id/index
    # to the added mesh.
    my $rect_id = Nemi::add_rect($renderbuf, 50, 50, 100, 100, 0x00FF00);

    # ^ The same rect can be updated with its id/index.
    Nemi::update_rect($renderbuf, $rect_id, 150, 150, 50, 50, 0xFF00FF);

    # When removing mesh from the render buffer,
    # The id/index gets flagged as "not used".
    # Then other mesh when added can use it again.
    # The index which was removed is saved for future add_{shape} calls.
    Nemi::remove_mesh($renderbuf, $rect_id);


    $vmode = vmode->new();

    my $term_rows = Nemi::term_get_rows();
    my $term_cols = Nemi::term_get_cols();
    print("(vmode) Terminal rows = $term_rows\n");
    print("(vmode) Terminal cols = $term_cols\n");
}


