#
# Nemi doesnt control the scrollback buffer offset from C code.
# but we can control it from perl script very easily.
# This script allows for user to control the scrollback buffer
# offset with keys.
# This was done to allow modifying the scrolling behaviour with ease.
#


use warnings;
use strict;


#!REGISTER_EVENT
sub event_key_input {
    print("key = $_[0], mods = $_[1]\n");
}

sub init_script {
    print("Init scrollback.pl\n");
}


