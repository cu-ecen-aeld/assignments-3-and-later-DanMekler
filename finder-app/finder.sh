#!/bin/sh

filesdir=$1
searchstr=$2

if [ -z "$filesdir" ] || [ -z "$searchstr" ]
then
    echo "Error: both arguments are required"
    exit 1
fi

if [ ! -d "$filesdir" ]
then
    echo "Error: first argument not a directory"
    exit 1
fi

files_num=$(find "$filesdir" -type f | wc -l)
string_num=$(grep -r -c "$searchstr" "$filesdir"/* | wc -l)

echo "The number of files are $files_num and the number of matching lines are $string_num"


