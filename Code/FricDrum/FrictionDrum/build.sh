#!/bin/bash

echo "build c"
gcc -c -o output/bcm2835.o include/bcm2835.c -O3

echo "build cpp"
g++ -c -o output/mpr121.o include/MPR121.cpp -O3
g++ -c -o output/i2cdev.o include/I2Cdev.cpp -O3

echo "build main.cpp"
g++ -c -o output/main.o main.cpp -std=c++0x  -O3 -Wall

echo "link"
g++ -o output/program output/bcm2835.o output/mpr121.o output/i2cdev.o output/main.o -lm -lSoundTouch -lpthread -lSDL2 -lwiringPi -lsndfile -O3 -Wall

