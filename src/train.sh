#!/bin/bash

epsilon=0.3
alpha=0.003
weight_name="wei2020.bin"
log_file="log2020.txt"

echo "1" > ${log_file}
./surakarta --total=100 --epsilon=${epsilon} --tuple="save=${weight_name}" >> ${log_file}

for i in $(seq 2 20)
do
    echo "\n${i}" >> ${log_file}
    epsilon=$(echo "scale=3;${epsilon} + 0.011" | bc)
    alpha=$(echo "scale=6;${alpha} * 0.93" | bc)
    ./surakarta --total=100 --epsilon=${epsilon} --tuple="load=${weight_name} save=${weight_name}" >> ${log_file}
done