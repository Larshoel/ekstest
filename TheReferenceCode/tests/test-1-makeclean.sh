#!/bin/bash

TEST=1-makeclean

cd /Users/larshoel/Documents/answers/15101
make clean 2>&1 > /tmp/OUTPUT-$TEST.txt
if [ $? -eq 0 ]
then
	echo OK
else
	echo "make clean returns $?"
fi

