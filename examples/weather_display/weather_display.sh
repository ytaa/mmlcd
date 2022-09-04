#!/bin/bash

mmlcdctl c

while :
do

    LOCATION=$(curl -s 'wttr.in/?format="%l"' | cut -c 2- | rev | cut -c 2- | rev)
    WEATHER=$(curl -s 'wttr.in/?format="%C+%t+%W"' | cut -c 2- | rev | cut -c 2- | rev | sed 's/°//g')
    WIND=$(curl -s 'wttr.in/?format="%w"' | cut -c 5- | rev | cut -c 2- | rev)
    mmlcdctl p "${LOCATION}    "
    mmlcdctl p "${WEATHER}     " 2

    sleep 10

done
