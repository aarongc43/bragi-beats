#!/bin/sh 

set -xe

CFLAGS="-Wall -Wextra -framework IOKit -framework Cocoa -framework OpenGL `pkg-config --libs --cflags raylib`"

cc $CFLAGS -o bragibeats main.c


