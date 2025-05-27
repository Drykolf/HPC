#!/bin/bash

for i in {1..10}
do
    ./matMulMPIo3 500
done
for i in {1..10}
do
    ./matMulMPIo3 1000
done
for i in {1..10}
do
    ./matMulMPIo3 1500
done
for i in {1..10}
do
    ./matMulMPIo3 2000
done
for i in {1..10}
do
    ./matMulMPIo3 3500
done
for i in {1..10}
do
    ./matMulMPIo3 5000
done