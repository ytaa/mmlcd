#!/bin/bash

mmlcdctl c
mmlcdctl p "CPU: "
mmlcdctl p "MEM: " 2

while :
do


    CPU=$(echo "CPU: "$[100-$(vmstat 1 2|tail -1|awk '{print $15}')]"%")
    MEM=$(free | grep Mem | awk '{print $3/$2 * 100.0}')
    MEM="MEM: ${MEM%.*}%"
    mmlcdctl p "${CPU}     "
    mmlcdctl p "${MEM}     " 2

done