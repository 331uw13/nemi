# About the project

You can create and modify perl scripts to customize the terminal emulator's behaviour
or add any kind of new features.

> [!WARNING]
> Scripting support in progress, Currently some functions may change or be removed without any warning. 
>
> > In the future, it will be continued with backward compatibility in mind.

## Documentation

* Working with scripts: https://github.com/331uw13/nemi/tree/main/documentation/scripts
* Development: https://github.com/331uw13/nemi/tree/main/documentation/development

# Build instructions
```bash
# Run this only once, it generates the Makefile
$ perl configure.pl

# If you need to use perlbrew.
$ perl configure.pl -using_perlbrew
```
```bash
# Build with compiler optimizations enabled and debug info disabled.
$ make release
```
```bash
# If you dont want to run 'install.sh'
$ ./nemi -home .
```


# Default scripts

  - ## `cmdl.pl`
    Alternative command line. You can evaluate perl expressions and the output is going to be written into echo terminal.
    Functions which are listed in the scripting documentation are also available. They start with the prefix `nemi::`
    
    Open `scripts/cmdl.pl` and check what is the toggle keybind. Then run `nemi::script_keybinds("cmdl")`.
    It will tell you all keybinds and what they do for the script. That command works for all loaded scripts which have keybinds assigned.


  - ## `vmode.pl`
    Vmode or "visual mode" creates another cursor for you. You can select text, interact with the word under the second cursor.
    And alot more, see more info with `nemi::script_keybinds("vmode")`


  - ## `scrollback.pl`
    This script was made to allow easy access to modify the scrolling behaviour.


  - ## `general.pl`
    Useful keybind shortcuts to functions.

# Troubleshooting

* First make sure logging is enabled from `configs/log.ini`. Then run it from different terminal emulator to see log output. 
You can also set the log output to a filepath, but make sure it can be accessed where you are running it from.


* To report bugs open an issue with corresponding label to your problem. Please include log output so logging can be improved :)


## Compatibility with older perl verions
If you are having problems with compiling the
project and are using older version of perl than 5.42.0
You can try to use perlbrew to use recent version.
```    
$ perlbrew init
$ perlbrew install perl-5.42.0
```
Then configure the project with
```
$ perl configure.pl -using_perlbrew
```

# Contribute
All kinds of ideas are welcome! Create pull request or open an issue and we can discuss about it.

----------------

### My contacts:
* Matrix.org: `@eeiuwie:matrix.org`
* Discord: `_331uw13`


