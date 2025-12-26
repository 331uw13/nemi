# Nemi - Terminal Emulator

## About the project
```
You can create and modify perl scripts to easily customize
the terminal emulator's behaviour
and to add any kind of new features.
>>> scripting support in progress! <<<

To load new perl script,
just add it into the `nemi.ini' [scripts] section,
and it should be automatically loaded.

`nemi.ini' is the configuration file for the program.


TODO: Add examples.
```


## Compatibility with older perl verions
```
If you are having problems with compiling the
project and are using older version of perl than 5.42.0.
You can try to use perlbrew to use recent version.
    
    perlbrew init
    perlbrew install perl-5.42.0

    Then configure the project with
    $ perl configure.pl -using_perlbrew
```


## Compiling the project
```
$ perl configure.pl && make
```

