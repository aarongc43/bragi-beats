#!/bin/sh

set -xe

# Set CFLAGS for include paths
CFLAGS="-Wall -Wextra -framework IOKit -framework Cocoa -framework OpenGL \
        -I/opt/homebrew/opt/raylib/include -Iraylib \
        -I/opt/homebrew/opt/mysql/include/mysql -Imysql\
        -I/opt/homebrew/opt/openssl@3/include -Iopenssl
        -I/*"

# Set LDFLAGS for library paths and to link against specific libraries
LDFLAGS="-L/opt/homebrew/opt/raylib/lib -lraylib \
         -L/opt/homebrew/opt/mysql/lib -lmysqlclient \
         -L/opt/homebrew/opt/zstd/lib -lzstd \
         -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto -lresolv"

# Compile and link in one step
cc $CFLAGS $LDFLAGS -o bragibeats main.c audioProcessing.c uiRendering.c visualizers.c
