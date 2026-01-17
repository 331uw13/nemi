# Nemi - Terminal Emulator
## About the project

You can create and modify perl scripts to customize the terminal emulator's behaviour
or add any kind of new features.

> [!NOTE]
> Scripting support in progress, Currently some functions may change or be removed without any warning.

## Documentation

* Working with scripts: https://github.com/331uw13/nemi/tree/main/documentation/scripts
* Development: https://github.com/331uw13/nemi/tree/main/documentation/development

## Compiling the project
```
$ perl configure.pl && make -j4
```

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

## Contribute
All kinds of ideas are welcome! Create pull request or open an issue and we can discuss about it.


----------------

### My contacts:
* Matrix.org: `@eeiuwie:matrix.org`
* Discord: `_331uw13`

