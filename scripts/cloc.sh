#!/bin/bash

if ! command -v cloc &> /dev/null
then
    echo "cloc could not be found"
    exit 1
fi

cloc ../src ../test ../include
