#!/usr/bin/env bash

if ! [ -d "./build" ]; then
    mkdir build
fi

cmake -S . -B build
cmake --build build
