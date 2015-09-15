#!/bin/bash

echo "build and link"
g++ -o output/program main.cpp -std=c++0x -Wall -lSoundTouch -lpthread -lSDL2 -lwiringPi -Wall -O3

echo "starting program"
sudo ./output/program
