#!/bin/bash

while :
do

    CURRENT_DATETIME=$(date | sed 'y/ęóąśćłżź/eoasclzz/') # replace special polish characters
    mmlcdctl p "${CURRENT_DATETIME:0:16}"
    mmlcdctl p " ${CURRENT_DATETIME:16:32}" 2
    sleep 0.5

done