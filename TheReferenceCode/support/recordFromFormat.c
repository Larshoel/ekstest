#include "recordFromFormat.h"
#include "recordToFormat.h"

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <ctype.h>
#include <arpa/inet.h>

Record* XMLtoRecord( char* buffer, int bufSize, int* bytesread )
{
    char* rec_end;

    if( buffer == NULL ) return NULL;

    // fprintf( stderr, "%s:%d entering XMLtoRecord with buffer %s\n", __FILE__, __LINE__, buffer );

    rec_end = strstr( buffer, "</record>" );

    if( rec_end == NULL )
    {
        *bytesread = 0;
        fprintf( stderr, "%s:%d leaving XMLtoRecord - no record\n", __FILE__, __LINE__ );
        return NULL;
    }

    rec_end += 9;
    *rec_end = 0; // overwrite > in closing tag with 0

    // fprintf( stderr, "%s:%d processing buffer %s\n", __FILE__, __LINE__, buffer );

    rec_end++;
    while( isspace(*rec_end) ) rec_end++; // discard tab, return, space etc

    *bytesread = rec_end - buffer;
    if( *bytesread > bufSize )
    {
        fprintf( stderr, "%s:%d ERROR: read %d bytes, more than buffer content %d\n", __FILE__, __LINE__, *bytesread, bufSize );
        exit( -1 );
    }

    char* search;
    Record* r = newRecord();
    initRecord( r );

    if( ( search = strstr( buffer, "<source=" ) ) != NULL )
    {
        char val = search[9];
        setSource( r, val );
    }

    if( ( search = strstr( buffer, "<dest=" ) ) != NULL )
    {
        char val = search[7];
        setDest( r, val );
    }

    if( ( search = strstr( buffer, "<username=" ) ) != NULL )
    {
        search += 11;
        char* tail = strstr( search, "\" />" );
        if( tail )
        {
            *tail = 0;
            setUsername( r, search );
            *tail = '\"';
        }
    }

    if( ( search = strstr( buffer, "<id=" ) ) != NULL )
    {
        uint32_t val;
        sscanf( search, "<id=\"%u\" />", &val );
        setId( r, val );
    }

    if( ( search = strstr( buffer, "<group=" ) ) != NULL )
    {
        uint32_t val;
        sscanf( search, "<group=\"%u\" />", &val );
        setGroup( r, val );
    }

    if( ( search = strstr( buffer, "<semester=" ) ) != NULL )
    {
        uint32_t val;
        sscanf( search, "<semester=\"%u\" />", &val );
        setSemester( r, val );
    }

    if( ( search = strstr( buffer, "<grade=\"PhD\" />" ) ) != NULL )
        setGrade( r, Grade_PhD );
    else if( ( search = strstr( buffer, "<grade=\"Master\" />" ) ) != NULL )
        setGrade( r, Grade_Master );
    else if( ( search = strstr( buffer, "<grade=\"Bachelor\" />" ) ) != NULL )
        setGrade( r, Grade_Bachelor );
    else if( ( search = strstr( buffer, "<grade=\"None\" />" ) ) != NULL )
        setGrade( r, Grade_None );

    if( ( search = strstr( buffer,"<course=\"IN1000\" />\n" ) ) != NULL ) setCourse( r, Course_IN1000 );
    if( ( search = strstr( buffer,"<course=\"IN1010\" />\n" ) ) != NULL ) setCourse( r, Course_IN1010 );
    if( ( search = strstr( buffer,"<course=\"IN1020\" />\n" ) ) != NULL ) setCourse( r, Course_IN1020 );
    if( ( search = strstr( buffer,"<course=\"IN1030\" />\n" ) ) != NULL ) setCourse( r, Course_IN1030 );
    if( ( search = strstr( buffer,"<course=\"IN1050\" />\n" ) ) != NULL ) setCourse( r, Course_IN1050 );
    if( ( search = strstr( buffer,"<course=\"IN1060\" />\n" ) ) != NULL ) setCourse( r, Course_IN1060 );
    if( ( search = strstr( buffer,"<course=\"IN1080\" />\n" ) ) != NULL ) setCourse( r, Course_IN1080 );
    if( ( search = strstr( buffer,"<course=\"IN1140\" />\n" ) ) != NULL ) setCourse( r, Course_IN1140 );
    if( ( search = strstr( buffer,"<course=\"IN1150\" />\n" ) ) != NULL ) setCourse( r, Course_IN1150 );
    if( ( search = strstr( buffer,"<course=\"IN1900\" />\n" ) ) != NULL ) setCourse( r, Course_IN1900 );
    if( ( search = strstr( buffer,"<course=\"IN1910\" />\n" ) ) != NULL ) setCourse( r, Course_IN1910 );

    // fprintf( stderr, "%s:%d leaving XMLtoRecord with record\n", __FILE__, __LINE__ );

    // printRecordAsXML( r );

    return r;
}

Record* BinaryToRecord( char* buffer, int bufSize, int* bytesread )
{
    int      min_length = 1;
    int      offset = 1;
    uint32_t namelen = 0;

    if( bufSize == 0 )
    {
        /* not even the flags have been received */
        *bytesread = 0;
        return NULL;
    }

    if( buffer[0] & FLAG_SRC ) min_length += 1;
    if( buffer[0] & FLAG_DST ) min_length += 1;

    if( buffer[0] & FLAG_USERNAME )
    {
        /* we can expect a user name, it is located after the flag byte
         * and after the src and dst bytes if those are present
         */
        if( bufSize < min_length+4 )
        {
            /* strlen is located in buffer[3-6], not enough data to hold it */
            *bytesread = 0;
            return NULL;
        }

        memcpy( &namelen, &buffer[min_length], 4 );
        namelen = htonl( namelen );

        min_length = min_length + 4 + namelen;
    }

    if( buffer[0] & FLAG_ID )       min_length += 4;
    if( buffer[0] & FLAG_GROUP )    min_length += 4;
    if( buffer[0] & FLAG_SEMESTER ) min_length += 1;
    if( buffer[0] & FLAG_GRADE )    min_length += 1;
    if( buffer[0] & FLAG_COURSES )  min_length += 2;

    if( bufSize < min_length )
    {
        *bytesread = 0;
        return NULL;
    }

    Record* r = newRecord();
    initRecord( r );

    if( buffer[0] & FLAG_SRC )
    {
        setSource( r, buffer[offset] );
        offset += 1;
    }

    if( buffer[0] & FLAG_DST )
    {
        setDest( r, buffer[offset] );
        offset += 1;
    }

    if( buffer[0] & FLAG_USERNAME )
    {
        uint32_t len;
        char*    s;
        memcpy( &len, &buffer[offset], 4 );
        offset += 4;
        len = htonl( len );
        s = malloc( len+1 );
        memcpy( s, &buffer[offset], len );
        offset += len;
        s[len] = 0;
        setUsername( r, s );
        free(s);
    }   

    if( buffer[0] & FLAG_ID )
    {
        uint32_t val;
        memcpy( &val, &buffer[offset], 4 );
        offset += 4;
        val = htonl(val);
        setId( r, val );
    }

    if( buffer[0] & FLAG_GROUP )
    {
        uint32_t val;
        memcpy( &val, &buffer[offset], 4 );
        offset += 4;
        val = htonl(val);
        setGroup( r, val );
    }

    if( buffer[0] & FLAG_SEMESTER )
    {
        uint8_t val;
        memcpy( &val, &buffer[offset], 1 );
        offset += 1;
        setSemester( r, val );
    }

    if( buffer[0] & FLAG_GRADE )
    {
        uint8_t val;
        memcpy( &val, &buffer[offset], 1 );
        offset += 1;
        setGrade( r, val );
    }

    if( buffer[0] & FLAG_COURSES )
    {
        uint16_t val;
        memcpy( &val, &buffer[offset], 2 );
        offset += 2;
        val = htons(val);
        setCourse( r, val );
    }

    *bytesread = offset;

    return r;
}

