use strict;
use warnings;


our $file_in;
our $file_out;
our $write_total;

sub appendf {
    #print($_[0]);
    print $file_out $_[0];
    $write_total += length($_[0]);
}

sub main {
    my $output_file = "src/register_script_functions.inc";
    my $input_file = "src/nemi_xs_wrappers.h";
    open($file_out, '>>', $output_file) or die $!;
    open($file_in, '<', $input_file) or die $!;

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

            appendf("newXS(\"nemi::$fn_name\", $xs_fn_name, __FILE__);\n");
        }
    }

    close($file_in);
    close($file_out);

    print("$write_total bytes was written to '$output_file'\n");
}


main();
