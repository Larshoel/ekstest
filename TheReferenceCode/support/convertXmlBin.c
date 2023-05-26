#include "recordToFormat.h"
#include "recordFromFormat.h"
#include "xmlfile.h"

#include <stdlib.h>
#include <string.h>
// #include <arpa/inet.h>

void usage( char* arg )
{
    fprintf( stderr, "Usage: %s <char>\n"
                     "       <char> is a capital ASCII letter, where <char>.bin will be converted to <char>.xml\n",
                     arg );
    exit( -1 );
}

void writeSleeptime( FILE* f, uint32_t sleeptime )
{
    sleeptime = htonl(sleeptime);
    fwrite( &sleeptime, 4, 1, f );
}

int main( int argc, char* argv[] )
{
    if( argc != 2 ) usage( argv[0] );

    char* srcname = strdup( "A.xml" );
    char* dstname = strdup( "A.bin" );

    char name = argv[1][0];
    if( name < 'A' || name > 'Z' ) usage( argv[0] );

    srcname[0] = name;
    dstname[0] = name;

    FILE* srcfile = xml_read_open( srcname );
    FILE* dstfile = fopen( dstname, "w" );

    int ct;

    do
    {
        char line[10000];

        uint32_t sleeptime;
        ct = xml_read( srcfile, line, 10000 );
        if( ct > 0 )
        {
            int err = sscanf( line, "<sleep=\"%u\" />", &sleeptime );
            if( err == 1 )
            {
                writeSleeptime( dstfile, sleeptime );
            }
            else
            {
                fprintf( stderr, "%s:%d read %d bytes from file\n", __FILE__, __LINE__, ct );
                int bytesread;
                Record* r = XMLtoRecord( line, ct, &bytesread );

                fprintRecordAsBinary( dstfile, r );
            }
        }
    }
    while( ct > 0 );

    fclose( srcfile );
    fclose( dstfile );

    return 0;
}

