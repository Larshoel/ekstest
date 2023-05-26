CFLAGS=-g -Wall -Wextra -std=gnu99

all: proxy xmlSender binSender anyReceiver makeTestfiles

#
# Linkning the 4 programs
#
proxy: proxy.o connection.o record.o recordToFormat.o recordFromFormat.o
	gcc $(CFLAGS) -o proxy proxy.o connection.o record.o recordToFormat.o recordFromFormat.o

xmlSender: xmlSender.o connection.o xmlfile.o
	gcc $(CFLAGS) -o xmlSender xmlSender.o connection.o xmlfile.o

binSender: binSender.o connection.o binfile.o
	gcc $(CFLAGS) -o binSender binSender.o connection.o binfile.o

anyReceiver: anyReceiver.o connection.o
	gcc $(CFLAGS) -o anyReceiver anyReceiver.o connection.o

makeTestfiles: makeTestfiles.o record.o recordToFormat.o
	gcc $(CFLAGS) -o makeTestfiles makeTestfiles.o record.o recordToFormat.o


makeTestfiles.o: makeTestfiles.c record.h
	gcc $(CFLAGS) -c $<

xmlSender.o: xmlSender.c connection.h xmlfile.h
	gcc $(CFLAGS) -c $<

binSender.o: binSender.c connection.h binfile.h
	gcc $(CFLAGS) -c $<

anyReceiver.o: anyReceiver.c connection.h
	gcc $(CFLAGS) -c $<

connection.o: connection.c connection.h
	gcc $(CFLAGS) -c $<

xmlfile.o: xmlfile.c xmlfile.h
	gcc $(CFLAGS) -c $<

binfile.o: binfile.c binfile.h record.h
	gcc $(CFLAGS) -c $<

record.o: record.c record.h
	gcc $(CFLAGS) -c $<

recordToFormat.o: recordToFormat.c recordToFormat.h record.h
	gcc $(CFLAGS) -c $<

recordFromFormat.o: recordFromFormat.c recordFromFormat.h record.h
	gcc $(CFLAGS) -c $<

proxy.o: proxy.c connection.h record.h
	gcc $(CFLAGS) -c $<

#
# Deleting the only object files
#
clean:
	rm -f *.o

#
# Deleting the object files and binary files
#
realclean: clean
	rm -f xmlSender
	rm -f binSender
	rm -f anyReceiver
	rm -f makeTestfiles
	rm -f proxy
	rm -f ?.xml
	rm -f ?.bin



