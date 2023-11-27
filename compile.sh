# MAC OSX systemssudo apt-get install build-essential
# install  build-essential and libsdl2-devel using apt, brew or whatever tool your *nix system uses

#!/bin/bash
gcc -c -fPIC perceptron.c -o perceptron.o -lm
gcc perceptron.o -shared -o libperceptron.so -lm
# sudo cp libperceptron.so /lib/
gcc universal.c -o universal -lm -L. -lperceptron
gcc test.c -o test -lm -L. -lperceptron


