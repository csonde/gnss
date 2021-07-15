#!/bin/bash

rm -rf ./bin
mkdir ./bin
g++ -g -o ./bin/run -I./incl ./src/*.cpp