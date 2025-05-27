#!/bin/bash

for i in {1..10}
do
    ./matMulMPI 500
done
for i in {1..10}
do
    ./matMulMPI 1000
done
for i in {1..10}
do
    ./matMulMPI 1500
done
for i in {1..10}
do
    ./matMulMPI 2000
done
for i in {1..10}
do
    ./matMulMPI 3500
done
for i in {1..10}
do
    ./matMulMPI 5000
done