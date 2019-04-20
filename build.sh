#!/bin/bash

toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android26-clang -I include -L lib  -ldiag  -lm -rpath lib  imei.c -o rwimei
