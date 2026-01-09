use strict;
use warnings;




our $file_in;
our $file_out;

sub appendf {
    print($_[0]);
    print $file_out $_[0];
}

sub main {
    open($file_out, '>>', "src/register_script_functions.inc") or die $!;
    open($file_in, '<', "src/nemi_xs_wrappers.h") or die $!;

    truncate $file_out, 0;

    appendf(
        "// GENERATED CODE. Changes here will be lost when project is compiled.\n" .
        "// Modify 'nemi_xs_wrappers.h' instead. Then run 'perl genxsfuncreg.pl'.\n"
    );

    while(my $line = <$file_in>) {
        
        if($line =~ m/^(XS\()/s) {

            my ($match) = $line =~ m/(\(.*\))/s;
            my $xs_fn_name = substr($match, 1, length($match)-2);
            my $fn_name    = substr($xs_fn_name, 4); 

            appendf("newXS(\"Nemi::$fn_name\", $xs_fn_name, __FILE__);\n");
        }

    }


    close($file_in);
    close($file_out);
}


main();
