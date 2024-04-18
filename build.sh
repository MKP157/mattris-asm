#!/bin/bash
gcc -o ./build/mattris ./main.c ./assembly/rotate.s -lncurses
echo "Build completed! Run with './build/mattris'."