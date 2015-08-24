#!/bin/bash

echo "build main.cpp"
g++ -c -o output/main.o main.cpp -std=c++0x -Wall -O3

echo "link"
g++ -o output/program output/main.o -lSoundTouch -lpthread -lSDL2 -lwiringPi -lsndfile -Wall -O3

