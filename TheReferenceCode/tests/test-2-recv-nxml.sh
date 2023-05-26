#!/bin/bash

TEST="2-recv-nxml"
PORT=45678

WAITING_TIME=6
OUPUT_PROXY_STDOUT="/tmp/OUTPUT-$TEST-po.txt"
OUTPUT_PROXY_STDERR="/tmp/OUTPUT-$TEST-pe.txt"
OUPUT_ANY_STDOUT="/tmp/OUTPUT-$TEST-co.txt"
OUTPUT_ANY_STDERR="/tmp/OUTPUT-$TEST-ce.txt"

( cd /Users/larshoel/Documents/TheReferenceCode/reference; make )
# /Users/larshoel/Documents/TheReferenceCode/reference/proxy $PORT &
/Users/larshoel/Documents/TheReferenceCode/reference/proxy $PORT \
	>$OUPUT_PROXY_STDOUT \
	2>$OUTPUT_PROXY_STDERR \
	&
PROXY_PID=$!
echo "OK - started proxy with PID $PROXY_PID"

/Users/larshoel/Documents/answers/15101/anyReceiver X X 127.0.0.1 $PORT 5 \
	>$OUPUT_ANY_STDOUT \
	2>$OUTPUT_ANY_STDERR \
	&
ANY_PID=$!
echo "OK - started anyReceiver with PID $ANY_PID, idle time 5 seconds"

ps -p $PROXY_PID 2>/dev/null >/dev/null
if [ $? -ne 0 ] ; then
	echo "ERROR - Proxy is not running - not the student's fault"
	exit -1
fi
ps -p $ANY_PID 2>/dev/null >/dev/null
if [ $? -ne 0 ] ; then
	echo "ERROR - anyReceiver is not running - perhaps the student's fault"
	exit -1
fi

sleep 1
echo "XA" > A.xml
cat Input/test-2-nxml.xml >> A.xml
nc 127.0.0.1 $PORT < A.xml
if [ $? -eq 0 ] ; then
	sleep 1
	echo "OK - nc finished without error"
else
	echo "ERROR - nc finished with error $?"
	exit -1
fi

diff -Bb X.xml Input/test-2-nxml.xml
if [ $? -eq 0 ] ; then
	echo "OK - sent and received XML files are identical"
	rm -f X.xml
	rm -f A.xml
else
	echo "ERROR - sent and received XML files are not identical"
	echo "This was received:"
	cat X.xml
fi

