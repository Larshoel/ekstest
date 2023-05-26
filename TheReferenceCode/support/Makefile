CFLAGS=-g -Wall -Wextra -std=gnu99

all: convertBinXml convertXmlBin

convertBinXml: convertBinXml.o binfile.o recordToFormat.o recordFromFormat.o record.o
	gcc $(CFLAGS) -o convertBinXml convertBinXml.o binfile.o recordToFormat.o recordFromFormat.o record.o

convertXmlBin: convertXmlBin.o xmlfile.o recordToFormat.o recordFromFormat.o record.o
	gcc $(CFLAGS) -o convertXmlBin convertXmlBin.o xmlfile.o recordToFormat.o recordFromFormat.o record.o


convertXmlBin.o: convertXmlBin.c binfile.h recordToFormat.h recordFromFormat.h
	gcc $(CFLAGS) -c $<

convertBinXml.o: convertBinXml.c binfile.h recordToFormat.h recordFromFormat.h
	gcc $(CFLAGS) -c $<

binfile.o: binfile.c binfile.h record.h
	gcc $(CFLAGS) -c $<

xmlfile.o: xmlfile.c xmlfile.h record.h
	gcc $(CFLAGS) -c $<

record.o: record.c record.h
	gcc $(CFLAGS) -c $<

recordToFormat.o: recordToFormat.c recordToFormat.h record.h
	gcc $(CFLAGS) -c $<

recordFromFormat.o: recordFromFormat.c recordFromFormat.h record.h
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
	rm -f convertBinXml
	rm -f convertXmlBin
	rm -f ?.xml
	rm -f ?.bin



