#!/bin/bash

CURRENT_DATETIME=$(date)
mmlcdctl p "${CURRENT_DATETIME:0:16}"
mmlcdctl p " ${CURRENT_DATETIME:18:32}" 2