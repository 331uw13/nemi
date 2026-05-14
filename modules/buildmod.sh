#!/bin/bash

# This is example script how to build modules.


module_name=$1
module_cfiles=$2

print_usage() {
    echo "c files can be separated by space."
    echo "usage: $0 <module_name.so> <c files>"
}

if [ -z $module_name ]; then
    print_usage
    exit
fi
if [ -z $module_cfiles ]; then
    print_usage
    exit
fi

set -xe

# The directory which libnemi.so lives, must be absolute.
libnemi_dir="$(cd .. && pwd)"
libnemi_srcdir="../src"

gcc $module_cfiles \
    -shared \
    -fPIC \
    -Wl,-rpath=$libnemi_dir \
    -L$libnemi_dir \
    -lnemi \
    -I$libnemi_srcdir \
    -o $module_name 




