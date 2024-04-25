#!/bin/bash
set -x

for i in $(seq 1 3);
do
    echo Version $i
    bash py_process.sh $i
done