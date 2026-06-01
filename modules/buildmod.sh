#!/bin/bash

#-----------------------------------------------
#  This is example script how to build modules
#-----------------------------------------------


module_name=$1
module_cfiles=$2

print_usage() {
    echo "usage: $0 <module name.so> <c file>"
}

if [ -z $module_name ]; then
    print_usage
    exit
fi
if [ -z $module_cfiles ]; then
    print_usage
    exit
fi


# This is just to prevent from accidentally overwriting the source code.
if [ -e $module_name ]; then
    is_elf=$(file $module_name | grep "ELF")

    if [ -z "$is_elf" ]; then
        echo "Oops. you probably got the arguments in wrong places."
        exit
    fi
fi

echo "module: $module_name"
echo "files: $module_cfiles"

set -xe


# The directory which libnemi.so lives, must be absolute.
libnemi_dir="$(cd .. && pwd)"
libnemi_srcdir="../src"


# This can be also set to "GRAPHICS_LINUX_FBDEV" in the future,
# to use the linux framebuffer device.
# But the implementation is not ready yet.
graphics_backend="GRAPHICS_OPENGL" 

gcc $module_cfiles \
    -D$graphics_backend \
    -shared \
    -fPIC \
    -Wl,-rpath=$libnemi_dir \
    -L$libnemi_dir \
    -lnemi \
    -I$libnemi_srcdir \
    -o $module_name 




