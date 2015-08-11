#!/bin/bash

echo "build c"
#gcc -c -o output/bcm2835.o include/bcm2835.c -I./include -O2

echo "build cpp"
g++ -c -o output/mpr121.o include/MPR121.cpp -I./include -O2
#g++ -c -o output/i2cdev.o include/I2Cdev.cpp -I./include  -O2

echo "build main.cpp"
g++ -c -I/usr/local/lib -o output/main.o main.cpp -I MPR  -std=c++0x -lpthread -lSDL2 -lwiringPi -O2

echo "link"
g++ -o output/program output/bcm2835.o output/mpr121.o output/i2cdev.o output/main.o -lSoundTouch -lpthread -lSDL2 -lwiringPi -O2

