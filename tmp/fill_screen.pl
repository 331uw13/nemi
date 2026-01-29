use strict;
use warnings;




sub main {

    my $stuff = "qwertyuiopasdfghjklzxcvbnm1234567890";

    for(my $i = 0; $i < 200; $i++) {
        for(my $j = 0; $j < 60; $j++) {
            print(substr($stuff, rand()*length($stuff), 1));
        }
    }
}


main();
