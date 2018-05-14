#!/bin/bash
gcc -c -fPIC perceptron.c -o perceptron.o # -lm
gcc perceptron.o -shared -o libperceptron.so # -lm
# sudo cp libperceptron.so /lib/
gcc universal.c -o universal -lm -L. -lperceptron
gcc test.c -o test -lm -L. -lperceptron


