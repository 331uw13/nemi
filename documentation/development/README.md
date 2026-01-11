# Development documentation


## How to create function which you can call from scripts

Open `src/nemi_xs_wrappers.h`. Then you will find some examples
which you can modify or copy and create another one.

After that you need to run `perl genxsfuncreg.pl` This will generate
the C code which registers functions for the scripts when Nemi loads them

Make sure to recompile the project after `make -B -j4`

