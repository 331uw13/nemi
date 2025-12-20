#
# Nemi doesnt control the scrollback buffer offset from C code.
# but we can control it from perl script very easily.
# This script allows for user to control the scrollback buffer
# offset with keys.
# This was done to allow modifying the scrolling behaviour with ease.
#



use warnings;
use strict;

sub event_char_input {}
sub event_key_input {}


sub init_script {
    print("Hello from scroll.pl\n");
}


