#!/bin/bash

TEST=1-make

cd /Users/larshoel/Documents/answers/15101
make 2>&1 > /tmp/OUTPUT-$TEST.txt
if [ $? -eq 0 ]
then
	echo OK
else
	echo "make returns $?"
fi

