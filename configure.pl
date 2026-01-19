use strict;
use warnings;

our $err_prefix   = " \033[1;31m(ERROR)\033[0m:";
our $ok_prefix    = " \033[1;32m(OK)\033[0m:";
our $info_prefix  = " \033[1;34m(INFO)\033[0m:";
use File::Find;
use Config;

our @installed_libs = ();
our $any_dependency_missing = 0;

our $libvterm_dir = "./libs/libvterm-0.3.3";
our $libvterm     = "$libvterm_dir/libvterm_l.a";


sub print_stage {
    print("====> $_[0]\n");
}

sub read_user_choice {
    print("$_[0]");
    my $test = <STDIN>;
    return uc(substr($test, 0, 1));
}

sub wanted {
    my $file = $File::Find::name;
    my $check = $file =~ m/(\.so|\.a)$/s; # Must end with '.so' or '.a'
    if($check) {
        push(@installed_libs, $file);
    }
}

sub get_installed_libs {
    my @search_dirs = ( 
        "/lib/", 
        "/lib64/",
        "/usr/lib/",
        "/usr/lib64/",
        "/usr/local/lib/"
    );

    find(\&wanted, @search_dirs);
}

sub argument_exists {
    foreach my $arg (@ARGV) {
        if($arg eq $_[0]) {
            print("\033[90m$arg\033[0m\n");
            return 1;
        }
    }
    return 0;
}


sub print_find_status {
    print($_[0]." ". "." x (28 - length($_[0]))." $_[1]\n");
}

sub check_lib_installed {
    if(grep(m/($_[0])/s, @installed_libs )) {
        print_find_status($_[0], "\033[1;32mFound\033[0m");
    }
    else {
        print_find_status($_[0], "\033[1;31mNot Found\033[0m");
        $any_dependency_missing = 1;
    }
}

our $FH;

sub f_append {
    print($_[0]);
    print $FH $_[0];
}

sub main {
    if(!argument_exists("-ignore_missing_libs")) {
        print_stage("Checking dependencies...");
        get_installed_libs();

        check_lib_installed("libGL.so");
        check_lib_installed("libGLEW.so");
        check_lib_installed("libglfw.so");
        check_lib_installed("libfreetype.so");
        check_lib_installed("libperl.so");
        
        if($any_dependency_missing) {
            print("\n\033[33m You can use '-ignore_missing_libs' if this is false assumption.\033[0m\n");
            exit();
        }
    }

    if(!-e $libvterm) {
        print("\n$info_prefix libvterm is not compiled.\n");
        my $choice = read_user_choice(" > Compile now? (y/n): ");
        if($choice ne 'Y') {
            exit();
        }
        
        system("(cd $libvterm_dir && make)");
        if(!-e $libvterm) {
            print("$err_prefix Failed to compile libvterm.");
            exit();
        }
        print("$ok_prefix libvterm compiled succesfully.\n");
    }

    my $perl_prefix = (argument_exists("-using_perlbrew") ? "perlbrew exec" : "perl");

    my @ccflags = ();
    my @ldflags = ();

    my $perl_dir = "$Config{archlib}/CORE";
    push(@ccflags, "-O2");
    push(@ccflags, "-Wall");
    push(@ccflags, "-Wextra");
    push(@ccflags, split / /,$Config{ccflags});
    push(@ccflags, "-I$perl_dir"); 
    push(@ccflags, "-I/usr/include/freetype2");
    push(@ccflags, "-I/usr/include/libpng16");
    push(@ccflags, "-I$libvterm_dir/include"); 
    push(@ccflags, "`$perl_prefix -MExtUtils::Embed -e ccopts -e ldopts`");
    push(@ldflags, "-lglfw");
    push(@ldflags, "-lGL");
    push(@ldflags, "-lGLEW");
    push(@ldflags, "-lm");
    push(@ldflags, "-lfreetype");
    push(@ldflags, "-lperl");
    push(@ldflags, "-lvterm_l");

    push(@ldflags, "-Wl,-rpath=$perl_dir -L$perl_dir");
    push(@ldflags, "-Wl,-rpath=$libvterm_dir -L$libvterm_dir");


    #print("------ CCFLAGS -----\n");
    #foreach my $flag (@ccflags) {
    #    print("$flag\n");
    #}


    #print("------ LDFLAGS -----\n");
    #foreach my $flag (@ldflags) {
    #    print("$flag\n");
    #}



    open($FH, '>>', "Makefile") or die $!;
    truncate $FH, 0;

    my $compiler = "gcc";
    

    my $target = "nemi";
    f_append("LIBNEMI_TARGET = lib$target.so\n");
    f_append("LOADER_TARGET = $target\n");

    f_append("CC = $compiler\n");
    f_append("CCFLAGS = \\\n");
    for(my $i = 0; $i < scalar(@ccflags); $i++) {
        my $flag = $ccflags[$i];
        f_append("        $flag ". ($i+1 >= scalar(@ccflags) ? "\n" : "\\\n"));
    }

    f_append("\n");
    f_append("LDFLAGS = \\\n");
    for(my $i = 0; $i < scalar(@ldflags); $i++) {
        my $flag = $ldflags[$i];
        f_append("        $flag ". ($i+1 >= scalar(@ldflags) ? "\n" : "\\\n"));
    }

    f_append(
        "LIB_SRC_DIR     = ./src\n" .
        "LOADER_SRC_DIR  = ./loader\n" .
        "OBJ_DIR         = ./obj\n" .
        "\n" .
        "\n" .
        "LIB_SRC_FILES = \$(shell find \$(LIB_SRC_DIR) -type f -name '*.c')\n" .
        "LIB_OBJ_FILES = \$(patsubst \$(LIB_SRC_DIR)/%.c, \$(OBJ_DIR)/%.o, \$(LIB_SRC_FILES))\n" .
        "\n" .
        "LOADER_SRC_FILES = \$(wildcard \$(LOADER_SRC_DIR)/*.c)\n" .
        "LOADER_OBJ_FILES = \$(patsubst \$(LOADER_SRC_DIR)/%.c, \$(OBJ_DIR)/%.o, \$(LOADER_SRC_FILES))\n" .
        "\n" .
        "\n" .
        "all: pre-build \$(LIBNEMI_TARGET) \$(LOADER_TARGET) post-build\n" .
        "\n" .
        "\n" .
        "pre-build:\n" .
        "\t\@$perl_prefix genxsfuncreg.pl\n" .
        "\n" .
        "post-build:\n" .
        "\t\@echo -e \"> \033[1;32mBuild complete!\033[0m\"\n" .
        "\n" .
        "\n" .
        "\n" .
        "# Compile and link library.\n" .
        "\n" .
        "\$(LIB_OBJ_FILES): \$(OBJ_DIR)/%.o: \$(LIB_SRC_DIR)/%.c\n" .
        "\t\@mkdir -p \$(shell dirname \$@)\n" .
        "\t\@echo \"\$<\"\n" .
        "\t@\$(CC) \$(CCFLAGS) -fPIC -c \$< -o \$@ || (echo -e \"\033[1;31mFailed to compile \$<\033[0m\")\n" .
        "\n" .
        "\$(LIBNEMI_TARGET): \$(LIB_OBJ_FILES)\n" .
        "\t@\$(CC) \$(LIB_OBJ_FILES) \$(LDFLAGS) -shared -o \$@\n" .
        "\n" .
        "\n" .
        "# Compile and link loader.\n" .
        "\n" .
        "\$(LOADER_OBJ_FILES): \$(OBJ_DIR)/%.o: \$(LOADER_SRC_DIR)/%.c\n" .
        "\t@\$(CC) \$(CCFLAGS) -c \$< -o \$@\n" .
        "\n" .
        "\$(LOADER_TARGET): \$(LOADER_OBJ_FILES)\n" .
        "\t@\$(CC) \$(LDFLAGS) \$(LOADER_OBJ_FILES) \$(OBJ_DIR)/string.o -o \$@\n" .
        "\n" .
        "\n" .
        "clean:\n" .
        "\t\@rm -v \$(LOADER_OBJ_FILES)\n" .
        "\t\@rm -v \$(LIB_OBJ_FILES)\n" .
        "\t\@rm -v \$(LOADER_TARGET)\n" .
        "\t\@rm -v src/register_script_functions.inc\n" .
        "\n" .
        ".PHONY: all clean\n"
    );

    print("$perl_prefix\n");

    
    close($FH);
    print("\n\033[1;32mMakefile was created.\033[0m\n");
}

main();

