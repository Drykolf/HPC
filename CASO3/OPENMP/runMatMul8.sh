#!/bin/bash

for size in 500 1000 1500 2000 3500 5000
do
    for i in {1..10} # repeticiones
    do
        ./matMul8 $size
    done
done