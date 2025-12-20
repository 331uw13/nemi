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
xsubpp version 3.51 (ExtUtils::ParseXS version 3.51)
Should _NOT_ be used,
because it has a bug where it doesnt keep variable names
from .xs files to generated C code. 
This obviously results into compile errors
Newer version should be used.

Replacing perl with new version on older system 
may break some dependencies.
`perlbrew' is recommended.
    
    perlbrew init
    perlbrew install perl-5.42.0
    

TODO: Create script to add variable names 
      to fix this issue.
```

## Compiling the project
```
$ perl configure.pl && make
```
* If you are using perlbrew:
```
$ perl configure.pl -using_perlbrew && make
```



