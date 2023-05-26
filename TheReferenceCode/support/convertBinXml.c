#include "recordToFormat.h"
#include "recordFromFormat.h"
#include "binfile.h"

#include <stdlib.h>
#include <string.h>
// #include <arpa/inet.h>

void usage( char* arg )
{
    fprintf( stderr, "Usage: %s [-s] <char>\n"
                     "       -s     a flag indicating that the input file has sleeptimes\n"
                     "       <char> is a capital ASCII letter, where <char>.bin will be converted to <char>.xml\n",
                     arg );
    exit( -1 );
}

void writeSleeptime( FILE* f, uint32_t sleeptime )
{
    fprintf( f, "<sleep=\"%u\" />\n\n", sleeptime );
}

int main( int argc, char* argv[] )
{
    if( argc != 2 && argc != 3 ) usage( argv[0] );

    bool has_sleeptime = false;

    if( argc == 3 )
    {
        if( strcmp(argv[1], "-s")==0 )
            has_sleeptime = true;
        else
            usage( argv[0] );
    }

    char* srcname = strdup( "A.bin" );
    char* dstname = strdup( "A.xml" );

    char name;
    if( argc == 3 ) name = argv[2][0];
               else name = argv[1][0];
    if( name < 'A' || name > 'Z' ) usage( argv[0] );

    srcname[0] = name;
    dstname[0] = name;

    BinaryFile* srcfile = bin_read_open( srcname );
    FILE* dstfile = fopen( dstname, "w" );

    int ct;

    do
    {
        if( has_sleeptime )
        {
            uint32_t sleeptime;
            ct = bin_read( srcfile, (char*)(&sleeptime), 4 );
            if( ct > 0 )
            {
                writeSleeptime( dstfile, sleeptime );
            }
        }

        char line[10000];
        ct = bin_read( srcfile, line, 10000 );
        if( ct > 0 )
        {
            int bytesread;
            Record* r = BinaryToRecord( line, ct, &bytesread );

            fprintRecordAsXML( dstfile, r );
        }
    }
    while( ct > 0 );

    bin_close( srcfile );

    fclose( dstfile );

    return 0;
}

