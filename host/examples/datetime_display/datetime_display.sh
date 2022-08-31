#!/bin/bash

CURRENT_DATETIME=$(date | sed 'y/ęóąśćłżź/eoasclzz/') # replace special polish characters
mmlcdctl p "${CURRENT_DATETIME:0:16}"
mmlcdctl p " ${CURRENT_DATETIME:18:32}" 2
