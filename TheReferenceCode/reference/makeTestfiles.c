#include "record.h"
#include "recordToFormat.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#undef BIGNAME
#undef SLEEP_ZERO

void usage( char* arg )
{
    fprintf( stderr, "Usage: %s [ <source> <type> <dests> <seed> ]\n"
                     "       <source> is a capital ASCII letter for the sender ID\n"
                     "       <type> is X for XML source or B for binary source\n"
                     "       <dests> is a sequence of capital letters for potential destination IDs\n"
                     "       <seed> is used to repeat random content generation\n"
                     "     A few default files are generated if no parameters are present.\n",
                     arg );
    exit( -1 );
}

void writeSleeptime( FILE* f, uint32_t sleeptime, char type )
{
    if( type == 'X' )
    {
        fprintf( f, "<sleep=\"%u\" />\n\n", sleeptime );
    }
    else
    {
        sleeptime = htonl(sleeptime);
        // fwrite( &sleeptime, 4, 1, f );
    }
}

int yes( )
{
    int retval = lrand48() % 2;
    return retval;
}

int main( int argc, char* argv[] )
{
    if( argc != 1 && argc != 5 ) usage( argv[0] );

    if( argc == 5 )
    {
        char  src;
        char  type;
        char* dst;
        int   num_dst;
        int   seed;
        char  filename[6];
        FILE* f;
        int   rounds;
        int   sleeptime;

        src = argv[1][0];
        if( src < 'A' || src > 'Z' ) usage( argv[0] );

        type = argv[2][0];
        if( type != 'X' && type != 'B' ) usage( argv[0] );

        dst = argv[3];
        num_dst = strlen( argv[3] );
        for( int i=0; i<num_dst; i++ )
        {
            if( dst[i] < 'A' || dst[i] > 'Z' ) usage( argv[0] );
        }

        seed = atoi( argv[4] );
        srand48( seed );

        if( type == 'X' ) sprintf( filename, "%c.xml", src );
                     else sprintf( filename, "%c.bin", src );

        f = fopen( filename, "w" );
        if( f == NULL )
        {
            fprintf( stderr, "Failed to open file %s\n", filename );
            exit( -1 );
        }

#ifdef SLEEP_ZERO
        if( type == 'X' ) fwrite( "XA", 2, 1, f );
                     else fwrite( "BA", 2, 1, f );
#endif

        rounds = lrand48() % 10;
        for( int i=0; i<rounds; i++ )
        {
#ifdef SLEEP_ZERO
            sleeptime = ( lrand48() % 3 );
#else
            sleeptime = ( lrand48() % 5 ) + 1;
#endif
            writeSleeptime( f, sleeptime, type );

            Record* r = newRecord();
            setSource( r, src );

            int rnd = lrand48() % num_dst;
            setDest( r, dst[rnd] );

            if( yes() )
            {
#ifdef BIGNAME
                char buffer[2001];
                int slen = lrand48() % 500;
                slen += 1500;
#else
                char buffer[21];
                int slen = lrand48() % 20;
                if( slen < 4 ) slen = 4;
#endif
                for( int j=0; j<slen; j++ )
                {
                    char b = 0;

                    while( ( b < 'a' || b > 'z' ) && ( b < 'A' || b > 'Z' ) )
                    {
                        b = (char)(lrand48() % 128);
                    }

                    buffer[j] = b;
                }
                buffer[slen] = 0;
                setUsername( r, buffer );
            }

            if( yes() ) setId( r, lrand48() % 10000 );
            if( yes() ) setGroup( r, lrand48() % 10000 );
            if( yes() ) setSemester( r, lrand48() % 30 );
            if( yes() ) setGrade( r, lrand48() % 4 );
            if( yes() )
                for( int j=0; j<11; j++ )
                    if( yes() )
                        setCourse( r, 1<<j );

            if( type == 'X' )
                fprintRecordAsXML( f, r );
            else
                fprintRecordAsBinary( f, r );

            deleteRecord( r );
        }
        
        fclose( f );
    }
    else
    {
        FILE* f = fopen( "A.xml", "w" );
        FILE* g = fopen( "B.bin", "w" );

        writeSleeptime( f, 1, 'X' );
        writeSleeptime( g, 1, 'B' );

        Record* r = newRecord();
        setSource(   r, 'A' );
        setDest(     r, 'D' );
        setUsername( r, "griff" );
        setId(       r, 1003 );
        setGroup(    r, 200 );
        setSemester( r, 27 );
        setGrade(    r, Grade_PhD );
        setCourse(   r, Course_IN1060 );
        setCourse(   r, Course_IN1020 );
        fprintRecordAsXML( f, r );
        setSource(   r, 'B' );
        fprintRecordAsBinary( g, r );

        writeSleeptime( f, 1, 'X' );
        writeSleeptime( g, 1, 'B' );

        clearRecord( r );
        setSource(   r, 'A' );
        setDest(     r, 'C' );
        setId(       r, 1 );
        setGroup(    r, 1 );
        setSemester( r, 07 );
        setGrade(    r, Grade_None );
        fprintRecordAsXML( f, r );
        setSource(   r, 'B' );
        fprintRecordAsBinary( g, r );

        writeSleeptime( f, 1, 'X' );
        writeSleeptime( g, 1, 'B' );

        clearRecord( r );
        setSource(   r, 'A' );
        setDest(     r, 'C' );
        setUsername( r, "john doe" );
        setId(       r, 1004 );
        setGroup(    r, 201 );
        setSemester( r, 2 );
        setGrade(    r, Grade_None );
        fprintRecordAsXML( f, r );
        setSource(   r, 'B' );
        fprintRecordAsBinary( g, r );

        writeSleeptime( f, 1, 'X' );
        writeSleeptime( g, 1, 'B' );

        clearRecord( r );
        setSource(   r, 'A' );
        setDest(     r, 'D' );
        setUsername( r, "hans hansen" );
        setId(       r, 1005 );
        setGroup(    r, 202 );
        setSemester( r, 8 );
        setGrade(    r, Grade_Bachelor );
        setCourse(   r, Course_IN1000 );
        setCourse(   r, Course_IN1020 );
        setCourse(   r, Course_IN1060 );
        setCourse(   r, Course_IN1140 );
        fprintRecordAsXML( f, r );
        setSource(   r, 'B' );
        fprintRecordAsBinary( g, r );

        deleteRecord( r );

        fclose( g );
        fclose( f );
    }

    return 0;
}

