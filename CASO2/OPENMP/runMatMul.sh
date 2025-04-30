#!/bin/bash

for i in {1..10}
do
    ./matMul8o 500
done
for i in {1..10}
do
    ./matMul8o 1000
done
for i in {1..10}
do
    ./matMul8o 1500
done
for i in {1..10}
do
    ./matMul8o 2000
done
for i in {1..10}
do
    ./matMul8o 3500
done
for i in {1..10}
do
    ./matMul8o 5000
done

for i in {1..10}
do
    ./matMul8o3 500
done
for i in {1..10}
do
    ./matMul8o3 1000
done
for i in {1..10}
do
    ./matMul8o3 1500
done
for i in {1..10}
do
    ./matMul8o3 2000
done
for i in {1..10}
do
    ./matMul8o3 3500
done
for i in {1..10}
do
    ./matMul8o3 5000
done