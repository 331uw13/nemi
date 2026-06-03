#!/bin/bash

# this will show all undefined symbols in "leaf".
# Leaf is the graphics wrapper and utility.

readelf --syms libnemi.so | awk '/0000000000000000/ && /leaf_/'
 
