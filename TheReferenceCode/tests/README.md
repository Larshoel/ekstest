# Testing code

The test files are mostly written in the shell script suitable for bash.
When you first see them, they may only be tested on Mac - I'll try to correct that.

The output of the commands is generated to the largest extent possible in files that
are called /tmp/OUTPUT-<testname>
When something goes wrong, you can look into those files to find out more.

## test-1

### test-1-make.sh

This checks if Make succeeds. When compilation for any of the files fails, it should return an ERROR string instead.
You will have to check /tmp/OUTPUT-1-make.txt to find the reason.

### test-1-makeclean.sh

This checks if "make clean" succeeds. Like above, OK should be printed in case of success, an ERROR string for
failure, and more info is found in /tmp/OUTPUT-1-makeclean.txt

### test-2-recv.sh

This is essentially a test for establishing communication between the proxy from the reference code and the
anyReceiver from the student's code. Effectively, this should be a test of the student's TCP code, which is
found in connection.c.
The initial test is only establishing and closing a connection.

### test-2-recv-1xml.sh

An extension of test-2-recv.sh, where nc is used to send a file containing one record.
Relies on an earlier successful test of test-2-recv.sh.
The test file should be identical to the output file apart from whitespace.

### test-2-recv-nxml.sh

An extension of test-2-recv.sh, where nc is used to send a file containing several records.
Relies on an earlier successful test of test-2-recv.sh.
The test file should be identical to the output file apart from whitespace.

