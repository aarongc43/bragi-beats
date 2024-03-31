#!/bin/sh

set -xe

# Set CFLAGS for include paths
CFLAGS="-Wall -Wextra -framework IOKit -framework Cocoa -framework OpenGL \
        -I/usr/local/opt/raylib/include \
        -I/usr/local/opt/mysql/include/mysql \
        -I/usr/local/opt/openssl@3/include"

# Set LDFLAGS for library paths and to link against specific libraries
LDFLAGS="-L/usr/local/opt/raylib/lib -lraylib \
         -L/usr/local/opt/mysql/lib -lmysqlclient \
         -L/usr/local/opt/zstd/lib -lzstd \
         -L/usr/local/opt/openssl@3/lib -lssl -lcrypto -lresolv"

# Compile and link in one step
cc $CFLAGS $LDFLAGS -o bragibeats main.c

