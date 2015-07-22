#!/bin/bash

echo "build c"
#gcc -c -o output/bcm2835.o include/bcm2835.c -I./include -g

echo "build cpp"
#g++ -c -o output/mpr121.o include/MPR121.cpp -I./include -g
#g++ -c -o output/i2cdev.o include/I2Cdev.cpp -I./include -g
#g++ -c -o output/wavfile.o include/WavFile.cpp -I./include -g

echo "build main.cpp"
g++ -c -I/usr/local/lib -o output/main.o main.cpp -I MPR  -std=c++0x -lpthread -g 

echo "link"
g++ -o output/program output/bcm2835.o output/mpr121.o output/i2cdev.o output/wavfile.o output/main.o -lpthread -g

