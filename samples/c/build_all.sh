#!/bin/sh
for i in *.c; do
    echo "compiling $i"
    g++ `pkg-config --cflags opencv` -o `basename $i .c` $i `pkg-config --libs opencv`;
done
for i in *.cpp; do
    echo "compiling $i"
    g++ `pkg-config --cflags opencv` -o `basename $i .cpp` $i `pkg-config --libs opencv`;
done

