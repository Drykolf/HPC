#!/bin/bash

for i in {1..10}
do
    ./matMul16o 500
done
for i in {1..10}
do
    ./matMul16o 1000
done
for i in {1..10}
do
    ./matMul16o 1500
done
for i in {1..10}
do
    ./matMul16o 2000
done
for i in {1..10}
do
    ./matMul16o 3500
done
for i in {1..10}
do
    ./matMul16o 5000
done

for i in {1..10}
do
    ./matMul16o3 500
done
for i in {1..10}
do
    ./matMul16o3 1000
done
for i in {1..10}
do
    ./matMul16o3 1500
done
for i in {1..10}
do
    ./matMul16o3 2000
done
for i in {1..10}
do
    ./matMul16o3 3500
done
for i in {1..10}
do
    ./matMul16o3 5000
done