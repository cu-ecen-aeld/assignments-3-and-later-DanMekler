#!/bin/bash

writefile=$1
writestr=$2

if [ -z "$writefile" ] || [ -z "$writestr" ]
then
    echo "Error: both arguments are required"
    exit 1
fi

dirpath=$(dirname "$writefile")

if [ ! -d "$dirpath" ]
then
    mkdir -p "$dirpath"
    echo "path created"
fi

echo "$writestr" > "$writefile"

