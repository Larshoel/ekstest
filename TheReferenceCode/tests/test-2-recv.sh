#!/bin/bash

TEST=2-recv

WAITING_TIME=6
OUPUT_PROXY_STDOUT=/tmp/OUTPUT-$TEST-po.txt
OUTPUT_PROXY_STDERR=/tmp/OUTPUT-$TEST-pe.txt
OUPUT_ANY_STDOUT=/tmp/OUTPUT-$TEST-co.txt
OUTPUT_ANY_STDERR=/tmp/OUTPUT-$TEST-ce.txt

( cd /Users/larshoel/Documents/TheReferenceCode/reference; make )
/Users/larshoel/Documents/TheReferenceCode/reference/proxy 45678 \
	>$OUPUT_PROXY_STDOUT \
	2>$OUTPUT_PROXY_STDERR \
	&
PROXY_PID=$!
echo "OK - started proxy with PID $PROXY_PID"

/Users/larshoel/Documents/answers/15101/anyReceiver Z X 127.0.0.1 45678 5 \
	>$OUPUT_ANY_STDOUT \
	2>$OUTPUT_ANY_STDERR \
	&
ANY_PID=$!
echo "OK - started anyReceiver with PID $ANY_PID, idle time 5 seconds"

ps -p $PROXY_PID 2>/dev/null >/dev/null
if [ $? -eq 0 ] ; then
	echo "OK - Proxy is running"
else
	echo "ERROR - Proxy is not running - not the student's fault"
	exit -1
fi
ps -p $ANY_PID 2>/dev/null >/dev/null
if [ $? -eq 0 ] ; then
	echo "OK - anyReceiver is running"
else
	echo "ERROR - anyReceiver is not running - perhaps the student's fault"
	exit -1
fi

echo "OK - waiting $WAITING_TIME seconds - allowing proxy to shut down"
sleep $WAITING_TIME

ps -p $ANY_PID 2>/dev/null >/dev/null
if [ $? -eq 0 ] ; then
	echo "ERROR - anyReceiver is still running - perhaps the student's fault"
        kill $ANY_PID
else
	echo "OK - anyReceiver is not running"
fi

ps -p $PROXY_PID 2>/dev/null >/dev/null
if [ $? -eq 0 ] ; then
	echo "ERROR - Proxy is still running - perhaps the student's fault"
	kill $PROXY_PID
else
	echo "OK - Proxy is not running"
fi

grep "accepted new TCP connection" $OUTPUT_PROXY_STDERR 2>/dev/null >/dev/null
if [ $? -eq 0 ] ; then
	echo "OK - anyReceiver connected successfully"
	grep "Client's ID is Z" $OUTPUT_PROXY_STDERR 2>/dev/null >/dev/null
	if [ $? -eq 0 ] ; then
		echo "OK - anyReceiver sent correct ID"
	else
		echo "ERROR - anyReceiver did not send correct ID - perhaps the student's fault"
	fi
else
	echo "ERROR - Proxy did not accept a connection - perhaps the student's fault"
fi

