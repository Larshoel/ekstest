#include "xmlfile.h"
#include "connection.h"
#include "record.h"
#include "recordToFormat.h"
#include "recordFromFormat.h"

#include <arpa/inet.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

enum ClientType
{
    CT_Undefined,
    CT_XML,
    CT_Binary
};

typedef enum ClientType ClientType;

/* This struct contain the information for one connected client.
 */
struct Client
{
    int            sock;
    char           id;
    ClientType     type;
    char*          buffer;
    int            offset;
    struct Client* next;
};

typedef struct Client Client;

int max( int l, int r )
{
    if( l >= r ) return l;
    return r;
}   

void usage( char* cmd )
{
    fprintf( stderr, "Usage: %s <port>\n"
                     "       This is the proxy server. It takes as imput the port where it accepts connections\n"
                     "       from \"xmlSender\", \"binSender\" and \"anyReceiver\" applications.\n"
                     "       <port> - a 16-bit integer in host byte order identifying the proxy server's port\n"
                     "\n",
                     cmd );
    exit( -1 );
}

/* The list of currently connected nodes.
 */
Client* clients = NULL;

/*
 * This function is called when a new connection is noticed on the server
 * socket.
 * The proxy accepts a new connection and creates the relevant data structures.
 */
void handleNewClient( int server_sock )
{
    int     conn_sock;
    Client* cl;
    Client* loop;

    fprintf( stderr, "%s:%d entering handleNewClient\n", __FILE__,__LINE__ );

    conn_sock = tcp_accept( server_sock );
    if( conn_sock >= 0 )
    {
        fprintf( stderr, "%s:%d accepted new TCP connection\n", __FILE__,__LINE__ );

        cl = malloc( sizeof( Client ) );
        if( cl == NULL )
        {
            fprintf( stderr, "%s:%d failed to allocate a new Client struct, exiting.\n", __FILE__,__LINE__ );
            exit( -1 );
        }

        cl->sock   = conn_sock;
        cl->id     = '\0';
        cl->type   = CT_Undefined;
        cl->next   = NULL;
        cl->offset = 0;

        cl->buffer = malloc( 10000 );
        if( cl->buffer == NULL )
        {
            fprintf( stderr, "%s:%d failed to allocate a new Client buffer, exiting.\n", __FILE__,__LINE__ );
            free( cl );
            exit( -1 );
        }

        if( clients == NULL )
        {
            clients = cl;
            fprintf( stderr, "%s:%d added the first client to the client list\n", __FILE__,__LINE__ );
        }
        else
        {
            loop = clients;
            while( loop->next != NULL ) loop = loop->next;
            loop->next = cl;
            fprintf( stderr, "%s:%d appended a client to the client list\n", __FILE__,__LINE__ );
        }
    }
}

/*
 * This function is called when a connection is broken by one of the connecting
 * clients. Data structures are clean up and resources that are no longer needed
 * are released.
 */
void removeClient( Client* client )
{
    close( client->sock );

    if( client == clients )
    {
        clients = clients->next;
        free( client->buffer );
        free( client );
    }
    else
    {
        Client* loop = clients;
        while( loop->next )
        {
            if( loop->next == client )
            {
                loop->next = client->next;
                free( client->buffer );
                free( client );
                return;
            }
            loop = loop->next;
        }
    }
}

/*
 * This function is called when the proxy received enough data from a sending
 * client to create a Record. The 'dest' field of the Record determines the
 * client to which the proxy should send this Record.
 *
 * If no such client is connected to the proxy, the Record is discarded without
 * error. Resources are released as appropriate.
 *
 * If such a client is connected, this functions find the correct socket for
 * sending to that client, and determines if the Record must be converted to
 * XML format or to binary format for sendig to that client.
 *
 * It does then send the converted messages.
 * Finally, this function deletes the Record before returning.
 */
void forwardMessage( Record* msg )
{
    fprintf( stderr, "%s:%d entering forwardMessage\n", __FILE__, __LINE__ );

    if( msg->has_dest )
    {
        fprintf( stderr, "%s:%d message has destination %c\n", __FILE__, __LINE__, msg->dest );

        Client* loop = clients;
        while( loop != NULL && loop->id != msg->dest )
        {
            loop = loop->next;
        }

        if( loop == NULL )
        {
            fprintf( stderr, "%s:%d destination %c not found\n", __FILE__, __LINE__, msg->dest );
        }
       else if( loop->id == msg->dest )
        {
            fprintf( stderr, "%s:%d destination %c found !\n", __FILE__, __LINE__, msg->dest );

            int err;
            char* buf;
            int bufSize;

            if( loop->type == CT_XML )
            {
                bufSize = 0;
                buf = recordToXML( msg, &bufSize );
                fprintf( stderr, "%s:%d convert to XML and sending %d bytes to %c\n", __FILE__, __LINE__, bufSize, msg->dest );
                err = tcp_write_loop( loop->sock, buf, bufSize );
                if( err < 0 )
                {
                    fprintf( stderr, "%s:%d TCP write failed for XML\n", __FILE__, __LINE__ );
                }
            }
            else if( loop->type == CT_Binary )
            {
                bufSize = 0;
                buf = recordToBinary( msg, &bufSize );
                fprintf( stderr, "%s:%d convert to Binary and sending %d bytes to %c\n", __FILE__, __LINE__, bufSize, msg->dest );
                err = tcp_write_loop( loop->sock, buf, bufSize );
                if( err < 0 )
                {
                    fprintf( stderr, "%s:%d TCP write failed for binary\n", __FILE__, __LINE__ );
                }
                free( buf );
            }
            else
            {
                fprintf( stderr, "%s:%d Destination client %c found but its type is not known. Not sending message.\n", __FILE__, __LINE__, loop->id );
            }
        }
    }
    else
    {
        fprintf( stderr, "%s:%d message has no destination\n", __FILE__, __LINE__ );
    }

    deleteRecord( msg );
}

/*
 * This function is called whenever activity is noticed on a connected socket,
 * and that socket is associated with a client. This can be sending client
 * or a receiving client.
 *
 * The calling function finds the Client structure for the socket where acticity
 * has occurred and calls this function.
 *
 * If this function receives data that completes a record, it creates an internal
 * Record data structure on the heap and calls forwardMessage() with this Record.
 *
 * If this function notices that a client has disconnected, it calls removeClient()
 * to release the resources associated with it.
 */
void handleClient( Client* client )
{
    int err;

    fprintf( stderr, "%s:%d handling client %c\n", __FILE__, __LINE__, client->id );

    if( client->type == CT_Undefined )
    {
        char type;
        err = tcp_read( client->sock, &type, 1 );
        if( err == 1 )
        {
            if( type == 'X' )
                client->type = CT_XML;
            else if( type == 'B' )
                client->type = CT_Binary;
            else
            {
                fprintf( stderr, "%s:%d Client's first byte does not identify XML or binary client. Ignoring byte.\n", __FILE__, __LINE__ );
            }
        }
        else if( err <= 0 )
        {
            /* Socket has been closed */
            fprintf( stderr, "%s:%d The socket %d has been closed before sending any bytes.\n", __FILE__, __LINE__, client->sock );
            removeClient( client );
        }
    }
    else if( client->id == 0 )
    {
        char id;
        err = tcp_read( client->sock, &id, 1 );
        if( err == 1 )
        {
            fprintf( stderr, "%s:%d Client's ID is %c\n", __FILE__, __LINE__, id );
            client->id = id;
        }
        else if( err <= 0 )
        {
            /* Socket has been closed */
            fprintf( stderr, "%s:%d The socket %d has been closed before sending its ID.\n", __FILE__, __LINE__, client->sock );
            removeClient( client );
        }
    }
    else
    {
        char* readptr;
        int bytesread;
        Record* msg = NULL;

        err = tcp_read( client->sock, &client->buffer[client->offset], 10000 - client->offset );
        if( err <= 0 )
        {
            /* Socket has been closed */
            fprintf( stderr, "%s:%d The socket %d (client %c) has been closed.\n", __FILE__, __LINE__, client->sock, client->id );
            removeClient( client );
            return;
        }
        fprintf( stderr, "%s:%d received %d bytes from socket %d\n", __FILE__, __LINE__, err, client->sock );

        client->offset += err;

        client->buffer[client->offset] = '\0';

        if( client->type == CT_XML )
        {
            do
            {
                readptr   = client->buffer;
                bytesread = 0;
                msg = XMLtoRecord( readptr, client->offset, &bytesread );
                if( msg )
                {
                    forwardMessage( msg );

                    readptr    += bytesread;
                    client->offset -= bytesread;
                    if( client->offset < 0 )
                    {
                        fprintf( stderr, "%s:%d ERROR: read more bytes than buffer contained %d, rest %d.\n", __FILE__, __LINE__, bytesread, client->offset );
                        client->offset = 0;
                    }

                    if( client->offset > 0 )
                    {
                        fprintf( stderr, "%s:%d more than one record in buffer, shifting %d bytes.\n", __FILE__, __LINE__, client->offset );
                        memcpy( client->buffer, readptr, client->offset );
                        client->buffer[ client->offset ] = 0;
                    }
                }
            }
            while( msg != NULL && client->offset > 0 );
        }
        else if( client->type == CT_Binary )
        {
            do
            {
                readptr   = client->buffer;
                bytesread = 0;
                msg = BinaryToRecord( readptr, client->offset, &bytesread );
                if( msg )
                {
                    forwardMessage( msg );

                    readptr    += bytesread;
                    client->offset -= bytesread;
                    if( client->offset < 0 )
                    {
                        fprintf( stderr, "%s:%d ERROR: read more bytes than buffer contained %d, rest %d.\n", __FILE__, __LINE__, bytesread, client->offset );
                        client->offset = 0;
                    }

                    if( client->offset > 0 )
                    {
                        fprintf( stderr, "%s:%d more than one record in buffer, shifting %d bytes.\n", __FILE__, __LINE__, client->offset );
                        memcpy( client->buffer, readptr, client->offset );
                    }
                }
            }
            while( msg != NULL && client->offset > 0 );
        }
        else
        {
            fprintf( stderr, "%s:%d Client has not been assigned a type (XML og binary)\n", __FILE__, __LINE__ );
        }
    }

    /* No problem if there is more data to read. Select will return again if the socket has more readable data.
     */
}

int main( int argc, char* argv[] )
{
    int port;
    int server_sock;
    int err;

    if( argc != 2 )
    {
        usage( argv[0] );
    }

    port = atoi( argv[1] );

    server_sock = tcp_create_and_listen( port );
    if( server_sock < 0 ) exit( -1 );
    
    /*
     * The following part is the event loop of the proxy. It waits for new connections,
     * new data arriving on existing connection, and events that indicate that a client
     * has disconnected.
     *
     * This function uses handleNewClient() when activity is seen on the server socket
     * and handleClient() when activity is seen on the socket of an existing connection.
     *
     * The loops ends when no clients are connected any more.
     */
    do
    {
        Client* loop;
        // Client* client;
        int     wait_end = 0;
        fd_set  waiting_set;

        FD_ZERO( &waiting_set );
        FD_SET( server_sock, &waiting_set );
        wait_end = server_sock + 1;

        loop = clients;
        while( loop != NULL )
        {
            FD_SET( loop->sock, &waiting_set );
            wait_end = max( wait_end, loop->sock+1 );
            loop = loop->next;
        }

        fprintf( stderr, "%s:%d entering event loop with upper fd bound %d\n", __FILE__, __LINE__, wait_end );

        err = tcp_wait( &waiting_set, wait_end );
        if( err >= 0 )
        {
            if( FD_ISSET( server_sock, &waiting_set ) )
            {
                fprintf( stderr, "%s:%d activity on server socket %d\n", __FILE__, __LINE__, server_sock );
                handleNewClient( server_sock );
            }
            
            loop = clients;
            while( loop != NULL )
            {
                Client* nextloop = loop->next;

                if( FD_ISSET( loop->sock, &waiting_set ) )
                {
                    fprintf( stderr, "%s:%d activity on client socket %d\n", __FILE__, __LINE__, loop->sock );
                    handleClient( loop );
                }

                loop = nextloop;
            }
        }
    }
    while( clients != NULL );

    tcp_close( server_sock );

    return 0;
}

