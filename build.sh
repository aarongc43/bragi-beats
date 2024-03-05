#!/bin/sh 

set -xe

CFLAGS="-Wall -Wextra -framework IOKit -framework Cocoa -framework OpenGL `pkg-config --libs --cflags raylib` `pkg-config --libs --cflags fftw3`"

cc $CFLAGS -o bragibeats main.c
