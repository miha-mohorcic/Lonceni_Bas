#!/bin/bash

echo "build c"
gcc -c -o output/bcm2835.o MPR/bcm2835.c -I MPR -g

echo "build cpp"
g++ -c -o output/mpr121.o MPR/MPR121.cpp -I MPR -g
g++ -c -o output/i2cdev.o MPR/I2Cdev.cpp -I MPR -g
g++ -c -o output/main.o main.cpp -I MPR -g

echo "link"
g++ -o output/program output/bcm2835.o output/mpr121.o output/i2cdev.o output/main.o -g

